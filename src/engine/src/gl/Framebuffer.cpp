#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Texture.hpp"
#include "engine/gl/Renderbuffer.hpp"

namespace engine::gl {

bool FramebufferCtx::hasInstances_{false};
GlHandle FramebufferCtx::contextFramebuffer_{GL_NONE};
GLenum FramebufferCtx::framebufferTarget_{GL_DRAW_FRAMEBUFFER};

FramebufferCtx::FramebufferCtx(Framebuffer const& useFramebuffer, bool bindAsDraw)
    : FramebufferCtx(useFramebuffer.Id(), bindAsDraw) { }

FramebufferCtx::FramebufferCtx(GLuint useFramebuffer, bool bindAsDraw) {
    // XLOG("FramebufferCtx ctor {}", useFramebuffer);
    assert(!hasInstances_);
    contextFramebuffer_.id = useFramebuffer;
    framebufferTarget_ = bindAsDraw ? GL_DRAW_FRAMEBUFFER : GL_READ_FRAMEBUFFER;
    GLCALL(glBindFramebuffer(framebufferTarget_, contextFramebuffer_.id));
    hasInstances_ = true;
}

FramebufferCtx::~FramebufferCtx() {
    // XLOG("FramebufferCtx dtor {}", contextFramebuffer_.id);
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    contextFramebuffer_.id = GL_NONE;
    GLCALL(glBindFramebuffer(framebufferTarget_, contextFramebuffer_.id));
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
    // assert(fb.IsComplete());
    return fb;
}

void Framebuffer::BindBackbuffer() { GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0U)); }

void Framebuffer::BindDraw() const { GLCALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbId_)); }

void Framebuffer::BindRead() const { GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, fbId_)); }

auto FramebufferCtx::ClearColor(GLint drawBufferIdx, GLint r, GLint g, GLint b, GLint a) -> FramebufferCtx&& {
    GLint rgba[] = {r, g, b, a};
    GLCALL(glClearBufferiv(GL_COLOR, drawBufferIdx, rgba));
    // 
    return std::move(*this);
}

auto FramebufferCtx::ClearColor(GLint drawBufferIdx, GLuint r, GLuint g, GLuint b, GLuint a) -> FramebufferCtx&& {
    GLuint rgba[] = {r, g, b, a};
    GLCALL(glClearBufferuiv(GL_COLOR, drawBufferIdx, rgba));
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
    bool updated = false;
    if (tex.IsTextureArray() || tex.IsCubemap()) {
        assert(arrayIndex >= 0);
        // NOTE: glFramebufferTexture is for Layered Framebuffers
        // glFramebufferTexture(GL_FRAMEBUFFER, attachment, tex.Id(), arrayIndex);
        GLCALL(glFramebufferTextureLayer(framebufferTarget_, attachment, tex.Id(), texLevel, arrayIndex));
        updated = true;
    }
    if (tex.Is1D()) {
        GLCALL(glFramebufferTexture1D(framebufferTarget_, attachment, GL_TEXTURE_1D, tex.Id(), texLevel));
        updated = true;
    }
    if (tex.Is2D()) {
        // NOTE: doesn't work for cubemaps, must use slice e.g.
        // GL_TEXTURE_CUBE_MAP_POSITIVE_X as texture type (target)
        GLCALL(glFramebufferTexture2D(framebufferTarget_, attachment, tex.TextureSlotTarget(), tex.Id(), texLevel));
        updated = true;
    }
    if (tex.Is3D()) {
        assert(arrayIndex >= 0);
        GLCALL(glFramebufferTexture3D(framebufferTarget_, attachment, GL_TEXTURE_3D, tex.Id(), texLevel, arrayIndex));
        updated = true;
    }
    if (updated) {
        assert(IsComplete());
    } else {
        assert(false && "Failed to set framebuffer attachment");
    }
    return std::move(*this);
}

auto FramebufferCtx::LinkRenderbuffer(GLenum attachment, Renderbuffer const& rb, GLint arrayIndex) -> FramebufferCtx&& {
    GLCALL(glFramebufferRenderbuffer(framebufferTarget_, attachment, rb.RenderbufferSlotTarget(), rb.Id()));
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
        GLCALL(glInvalidateFramebuffer(framebufferTarget_, numAttachments, attachments));
    }
    return std::move(*this);
}

auto Framebuffer::IsComplete() -> bool {
    bool isComplete = false;
    GLCALL(isComplete = glCheckFramebufferStatus(fbId_) == GL_FRAMEBUFFER_COMPLETE);
    if (isComplete) {
        LogDebugLabel(*this, "Framebuffer is not complete");
    }
    return isComplete;
}

auto FramebufferCtx::IsComplete() -> bool {
    bool isComplete = false;
    GLCALL(isComplete = glCheckFramebufferStatus(framebufferTarget_) == GL_FRAMEBUFFER_COMPLETE);
    if (isComplete) {
        LogDebugLabelUnsafe(contextFramebuffer_.id, GlObjectType::FRAMEBUFFER, "Framebuffer is not complete");
    }
    return isComplete;
}

} // namespace engine::gl
