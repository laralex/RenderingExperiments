#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Texture.hpp"

namespace engine::gl {

bool FramebufferCtx::hasInstances_{false};
GlHandle FramebufferCtx::contextFramebuffer_{GL_NONE};
bool FramebufferCtx::isDrawBinding_{false};

FramebufferCtx::FramebufferCtx(Framebuffer const& useFramebuffer, bool bindAsDraw)
    : FramebufferCtx(useFramebuffer.Id(), bindAsDraw) { }

FramebufferCtx::FramebufferCtx(GLuint useFramebuffer, bool bindAsDraw) {
    assert(!hasInstances_);
    contextFramebuffer_.id = useFramebuffer;
    isDrawBinding_         = bindAsDraw;
    GLCALL(glBindFramebuffer(bindAsDraw ? GL_DRAW_FRAMEBUFFER : GL_READ_FRAMEBUFFER, contextFramebuffer_.id));
    hasInstances_ = true;
}

FramebufferCtx::~FramebufferCtx() {
    assert(hasInstances_);
    contextFramebuffer_.id = GL_NONE;
    GLCALL(glBindVertexArray(contextFramebuffer_.id));
    hasInstances_ = false;
}

void Framebuffer::Dispose() {
    if (fbId_ == GL_NONE) { return; }
    LogDebugLabel(*this, "Framebuffer object was disposed");
    GLCALL(glDeleteFramebuffers(1, &fbId_.id));
    fbId_.id = GL_NONE;
}

auto Framebuffer::Allocate(std::string_view name) -> Framebuffer {
    Framebuffer fb{};
    GLCALL(glGenFramebuffers(1, &fb.fbId_.id));
    fb.BindRead();
    if (!name.empty()) {
        DebugLabel(fb, name);
        LogDebugLabel(fb, "Framebuffer was allocated");
    }
    return fb;
}

void Framebuffer::BindBackbuffer() { GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0U)); }

void Framebuffer::BindDraw() const { GLCALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbId_)); }

void Framebuffer::BindRead() const { GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, fbId_)); }

auto FramebufferCtx::ClearColor(GLint drawBufferIdx, GLint r, GLint g, GLint b, GLint a) -> FramebufferCtx&& {
    GLint rgba[] = {r, g, b, a};
    GLCALL(glClearBufferiv(GL_FRAMEBUFFER, drawBufferIdx, rgba));
    // if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    return std::move(*this);
}

auto FramebufferCtx::ClearColor(GLint drawBufferIdx, GLuint r, GLuint g, GLuint b, GLuint a) -> FramebufferCtx&& {
    GLuint rgba[] = {r, g, b, a};
    GLCALL(glClearBufferuiv(GL_FRAMEBUFFER, drawBufferIdx, rgba));
    return std::move(*this);
}

auto FramebufferCtx::ClearColor(GLint drawBufferIdx, GLfloat r, GLfloat g, GLfloat b, GLfloat a) -> FramebufferCtx&& {
    GLfloat rgba[] = {r, g, b, a};
    GLCALL(glClearBufferfv(GL_COLOR, drawBufferIdx, rgba));
    return std::move(*this);
}

auto FramebufferCtx::ClearDepth(GLfloat value) -> FramebufferCtx&& {
    GLCALL(glClearBufferfv(GL_DEPTH, 0, &value));
    return std::move(*this);
}

auto FramebufferCtx::ClearStencil(GLint value) -> FramebufferCtx&& {
    GLCALL(glClearBufferiv(GL_STENCIL, 0, &value));
    return std::move(*this);
}

auto FramebufferCtx::ClearDepthStencil(GLfloat depth, GLint stencil) -> FramebufferCtx&& {
    GLCALL(glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil));
    return std::move(*this);
}

auto FramebufferCtx::LinkTexture(GLenum attachment, Texture const& tex, GLint texLevel, GLint arrayIndex)
    -> FramebufferCtx&& {
    assert(
        attachment >= GL_COLOR_ATTACHMENT0 && attachment <= GL_COLOR_ATTACHMENT31 || attachment == GL_DEPTH_ATTACHMENT
        || attachment == GL_STENCIL_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT);
    if (tex.IsTextureArray() || tex.IsCubemap()) {
        assert(arrayIndex >= 0);
        // NOTE: glFramebufferTexture is for Layered Framebuffers
        // glFramebufferTexture(GL_FRAMEBUFFER, attachment, tex.Id(), arrayIndex);
        GLCALL(glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, tex.Id(), texLevel, arrayIndex));
        return std::move(*this);
    }
    if (tex.Is1D()) {
        GLCALL(glFramebufferTexture1D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_1D, tex.Id(), texLevel));
        return std::move(*this);
    }
    if (tex.Is2D()) {
        // NOTE: doesn't work for cubemaps, must use slice e.g.
        // GL_TEXTURE_CUBE_MAP_POSITIVE_X as texture type (target)
        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, tex.TextureType(), tex.Id(), texLevel));
        return std::move(*this);
    }
    if (tex.Is3D()) {
        assert(arrayIndex >= 0);
        GLCALL(glFramebufferTexture3D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_3D, tex.Id(), texLevel, arrayIndex));
        return std::move(*this);
    }
    assert(false && "Failed to set framebuffer attachment");
    return std::move(*this);
}

// auto FramebufferCtx::LinkBackbuffer(GLenum attachment, GLint texLevel) -> FramebufferCtx&& {
//     assert(attachment >= GL_COLOR_ATTACHMENT0 && attachment <= GL_COLOR_ATTACHMENT31);
//     GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, texLevel));
// return std::move(*this);
// }

// valid attachments: COLOR_ATTACHMENTi, DEPTH_ATTACHMENT, or STENCIL_ATTACHMENT
auto FramebufferCtx::Invalidate(uint32_t numAttachments, GLenum* attachments) -> FramebufferCtx&& {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::ARB_invalidate_subdata)) {
        GLCALL(glInvalidateFramebuffer(GL_FRAMEBUFFER, numAttachments, attachments));
    }
    return std::move(*this);
}
} // namespace engine::gl
