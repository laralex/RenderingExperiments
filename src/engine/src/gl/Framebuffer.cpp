#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Renderbuffer.hpp"
#include "engine/gl/Texture.hpp"

namespace engine::gl {

bool FramebufferDrawCtx::hasInstances_{false};
GlHandle FramebufferDrawCtx::contextFramebuffer_{GL_NONE};
GLenum FramebufferDrawCtx::framebufferTarget_{GL_DRAW_FRAMEBUFFER};

FramebufferDrawCtx::FramebufferDrawCtx(Framebuffer const& useFramebuffer, bool bindAsDraw) noexcept
    : FramebufferDrawCtx(useFramebuffer.Id(), bindAsDraw) { }

FramebufferDrawCtx::FramebufferDrawCtx(GLuint useFramebuffer, bool bindAsDraw) noexcept {
    // XLOG("FramebufferCtx ctor {}", useFramebuffer);
    assert(!hasInstances_ && "FramebufferDrawCtx another already exists in the scope");
    GuardAnother(useFramebuffer, bindAsDraw);
    hasInstances_ = true;
}



FramebufferDrawCtx::~FramebufferDrawCtx() noexcept {
    // XLOG("FramebufferCtx dtor {}", contextFramebuffer_.id);
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    contextFramebuffer_.UnsafeReset();
    GLCALL(glBindFramebuffer(framebufferTarget_, 0U));
    hasInstances_ = false;
}

void FramebufferDrawCtx::GuardAnother(GLuint useFramebuffer, bool bindAsDraw) noexcept {
    // safe, because FramebufferDrawCtx doesn't own any resources, no leaking
    contextFramebuffer_.UnsafeAssign(GlHandle{useFramebuffer});
    framebufferTarget_  = bindAsDraw ? GL_DRAW_FRAMEBUFFER : GL_READ_FRAMEBUFFER;
    GLCALL(glBindFramebuffer(framebufferTarget_, contextFramebuffer_));
}

Framebuffer::Framebuffer() noexcept { std::fill(std::begin(drawBuffers_), std::end(drawBuffers_), GL_NONE); }

void Framebuffer::Dispose() {
    if (fbId_ == GL_NONE) { return; }
    LogDebugLabel(*this, "Framebuffer object was disposed");
    GLCALL(glDeleteFramebuffers(1, &fbId_));
    fbId_.UnsafeReset();
}

auto Framebuffer::Allocate(std::string_view name) -> Framebuffer {
    Framebuffer fb{};
    GLCALL(glGenFramebuffers(1, &fb.fbId_));
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

auto FramebufferDrawCtx::ClearColor(GLint drawBufferIdx, GLint r, GLint g, GLint b, GLint a) const
    -> FramebufferDrawCtx const& {
    GLint rgba[] = {r, g, b, a};
    GLCALL(glClearBufferiv(GL_COLOR, drawBufferIdx, rgba));
    return *this;
}

auto FramebufferDrawCtx::ClearColor(GLint drawBufferIdx, GLuint r, GLuint g, GLuint b, GLuint a) const
    -> FramebufferDrawCtx const& {
    GLuint rgba[] = {r, g, b, a};
    GLCALL(glClearBufferuiv(GL_COLOR, drawBufferIdx, rgba));
    return *this;
}

auto FramebufferDrawCtx::ClearColor(GLint drawBufferIdx, GLfloat r, GLfloat g, GLfloat b, GLfloat a) const
    -> FramebufferDrawCtx const& {
    GLfloat rgba[] = {r, g, b, a};
    GLCALL(glClearBufferfv(GL_COLOR, drawBufferIdx, rgba));
    return *this;
}

auto FramebufferDrawCtx::ClearDepth(GLfloat value) const -> FramebufferDrawCtx const& {
    GLCALL(glClearBufferfv(GL_DEPTH, 0, &value));
    return *this;
}

auto FramebufferDrawCtx::ClearStencil(GLint value) const -> FramebufferDrawCtx const& {
    GLCALL(glClearBufferiv(GL_STENCIL, 0, &value));
    return *this;
}

auto FramebufferDrawCtx::ClearDepthStencil(GLfloat depth, GLint stencil) const -> FramebufferDrawCtx const& {
    GLCALL(glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil));
    return *this;
}

// auto FramebufferDrawCtx::LinkBackbuffer(GLenum attachment, GLint texLevel) -> FramebufferDrawCtx&& {
//     assert(attachment >= GL_COLOR_ATTACHMENT0 && attachment <= GL_COLOR_ATTACHMENT31);
//     GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, texLevel));
// return *this;
// }

// valid attachments: COLOR_ATTACHMENTi, DEPTH_ATTACHMENT, or STENCIL_ATTACHMENT
auto FramebufferDrawCtx::Invalidate(CpuMemory<GLenum const> attachments) const -> FramebufferDrawCtx const& {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::ARB_invalidate_subdata)) {
        GLCALL(glInvalidateFramebuffer(framebufferTarget_, attachments.NumElements(), attachments[0]));
    }
    return *this;
}

auto FramebufferDrawCtx::IsComplete() const -> bool {
    bool isComplete = false;
    GLCALL(isComplete = glCheckFramebufferStatus(framebufferTarget_) == GL_FRAMEBUFFER_COMPLETE);
    if (!isComplete) {
        LogDebugLabelUnsafe(contextFramebuffer_, GlObjectType::FRAMEBUFFER, "Framebuffer is not complete");
    }
    return isComplete;
}

FramebufferEditCtx::FramebufferEditCtx(Framebuffer& useFramebuffer, bool bindAsDraw) noexcept
    : ctx_{useFramebuffer, bindAsDraw}
    , fb_{useFramebuffer} { }

auto FramebufferEditCtx::AttachTexture(GLenum attachment, Texture const& tex, GLint texLevel, GLint arrayIndex) const
    -> FramebufferEditCtx const& {
    assert(
        attachment >= GL_COLOR_ATTACHMENT0 && attachment <= GL_COLOR_ATTACHMENT31 || attachment == GL_DEPTH_ATTACHMENT
        || attachment == GL_STENCIL_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT);
    bool updated           = true;
    auto framebufferTarget = ctx_.BoundTarget();
    if (tex.Is2D()) {
        // NOTE: doesn't work for cubemaps, must use slice e.g.
        // GL_TEXTURE_CUBE_MAP_POSITIVE_X as texture type (target)
        GLCALL(glFramebufferTexture2D(framebufferTarget, attachment, tex.TextureSlotTarget(), tex.Id(), texLevel));
    } else if (tex.IsTextureArray() || tex.IsCubemap()) {
        assert(arrayIndex >= 0);
        // NOTE: glFramebufferTexture is for Layered Framebuffers
        // glFramebufferTexture(GL_FRAMEBUFFER, attachment, tex.Id(), arrayIndex);
        GLCALL(glFramebufferTextureLayer(framebufferTarget, attachment, tex.Id(), texLevel, arrayIndex));
    } else if (tex.Is1D()) {
        GLCALL(glFramebufferTexture1D(framebufferTarget, attachment, GL_TEXTURE_1D, tex.Id(), texLevel));
    } else if (tex.Is3D()) {
        assert(arrayIndex >= 0);
        GLCALL(glFramebufferTexture3D(framebufferTarget, attachment, GL_TEXTURE_3D, tex.Id(), texLevel, arrayIndex));
    } else {
        updated = false;
    }

    if (updated) {
        // can be nullptr for backbuffer
        int32_t const drawBuffer      = attachment - GL_COLOR_ATTACHMENT0;
        fb_.drawBuffers_[drawBuffer] = attachment;
        assert(IsComplete());
    } else {
        assert(false && "Failed to set framebuffer attachment");
    }
    return *this;
}

auto FramebufferEditCtx::AttachRenderbuffer(GLenum attachment, Renderbuffer const& rb, GLint arrayIndex) const
    -> FramebufferEditCtx const& {
    GLCALL(glFramebufferRenderbuffer(ctx_.BoundTarget(), attachment, rb.RenderbufferSlotTarget(), rb.Id()));
    int32_t const drawBuffer      = attachment - GL_COLOR_ATTACHMENT0;
    fb_.drawBuffers_[drawBuffer] = attachment;
    return *this;
}

auto FramebufferEditCtx::CommitDrawbuffers() const -> FramebufferEditCtx const& {
    assert(ctx_.BoundTarget() == GL_DRAW_FRAMEBUFFER);
    GLCALL(glDrawBuffers(static_cast<GLsizei>(Framebuffer::MAX_DRAW_BUFFERS), fb_.drawBuffers_));
    return *this;
}

auto FramebufferEditCtx::SetReadbuffer(GLenum attachment) const -> FramebufferEditCtx const& {
    assert(ctx_.BoundTarget() == GL_READ_FRAMEBUFFER);
    GLCALL(glReadBuffer(attachment));
    return *this;
}

} // namespace engine::gl
