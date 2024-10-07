#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Renderbuffer.hpp"
#include "engine/gl/Texture.hpp"

#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_STATIC bool FramebufferDrawCtx::hasInstances_{false};
ENGINE_STATIC GlHandle FramebufferDrawCtx::contextFramebuffer_{GL_NONE};
ENGINE_STATIC GLenum FramebufferDrawCtx::framebufferTarget_{GL_DRAW_FRAMEBUFFER};

ENGINE_EXPORT FramebufferDrawCtx::FramebufferDrawCtx(Framebuffer const& useFramebuffer, bool bindAsDraw) noexcept
    : FramebufferDrawCtx(useFramebuffer.Id(), bindAsDraw) { }

ENGINE_EXPORT FramebufferDrawCtx::FramebufferDrawCtx(GLuint useFramebuffer, bool bindAsDraw) noexcept {
    // XLOG("FramebufferCtx ctor {}", useFramebuffer);
    assert(!hasInstances_ && "FramebufferDrawCtx another already exists in the scope");
    GuardAnother(useFramebuffer, bindAsDraw);
    hasInstances_ = true;
}

ENGINE_EXPORT FramebufferDrawCtx::~FramebufferDrawCtx() noexcept {
    // XLOG("FramebufferCtx dtor {}", contextFramebuffer_.id);
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    contextFramebuffer_.UnsafeReset();
    GLCALL(glBindFramebuffer(framebufferTarget_, 0U));
    hasInstances_ = false;
}

ENGINE_EXPORT void FramebufferDrawCtx::GuardAnother(GLuint useFramebuffer, bool bindAsDraw) noexcept {
    // safe, because FramebufferDrawCtx doesn't own any resources, no leaking
    contextFramebuffer_.UnsafeAssign(useFramebuffer);
    framebufferTarget_ = bindAsDraw ? GL_DRAW_FRAMEBUFFER : GL_READ_FRAMEBUFFER;
    GLCALL(glBindFramebuffer(framebufferTarget_, contextFramebuffer_));
}

ENGINE_EXPORT Framebuffer::Framebuffer() noexcept {
    std::fill(std::begin(drawBuffers_), std::end(drawBuffers_), GL_NONE);
}

ENGINE_EXPORT void Framebuffer::Dispose() {
    if (fbId_ == GL_NONE) { return; }
    // LogDebugLabel(*this, "Framebuffer object was disposed");
    XLOG("Framebuffer object was disposed: 0x{:08X}", GLuint(fbId_));
    GLCALL(glDeleteFramebuffers(1, fbId_.Ptr()));
    fbId_.UnsafeReset();
}

ENGINE_EXPORT auto Framebuffer::Allocate(GlContext& gl, std::string_view name) -> Framebuffer {
    Framebuffer fb{};
    GLCALL(glGenFramebuffers(1, fb.fbId_.Ptr()));
    fb.BindRead();
    if (!name.empty()) {
        DebugLabel(gl, fb, name);
        LogDebugLabel(gl, fb, "Framebuffer was allocated");
    }
    // assert(fb.IsComplete());
    return fb;
}

ENGINE_EXPORT void Framebuffer::BindBackbuffer() { GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0U)); }

ENGINE_EXPORT void Framebuffer::BindDraw() const { GLCALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbId_)); }

ENGINE_EXPORT void Framebuffer::BindRead() const { GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, fbId_)); }

ENGINE_EXPORT auto FramebufferDrawCtx::ClearColor(GLint drawBufferIdx, GLint r, GLint g, GLint b, GLint a) const
    -> FramebufferDrawCtx const& {
    GLint rgba[] = {r, g, b, a};
    GLCALL(glClearBufferiv(GL_COLOR, drawBufferIdx, rgba));
    return *this;
}

ENGINE_EXPORT auto FramebufferDrawCtx::ClearColor(GLint drawBufferIdx, GLuint r, GLuint g, GLuint b, GLuint a) const
    -> FramebufferDrawCtx const& {
    GLuint rgba[] = {r, g, b, a};
    GLCALL(glClearBufferuiv(GL_COLOR, drawBufferIdx, rgba));
    return *this;
}

ENGINE_EXPORT auto FramebufferDrawCtx::ClearColor(GLint drawBufferIdx, GLfloat r, GLfloat g, GLfloat b, GLfloat a) const
    -> FramebufferDrawCtx const& {
    GLfloat rgba[] = {r, g, b, a};
    GLCALL(glClearBufferfv(GL_COLOR, drawBufferIdx, rgba));
    return *this;
}

ENGINE_EXPORT auto FramebufferDrawCtx::ClearDepth(GLfloat value) const -> FramebufferDrawCtx const& {
    GLCALL(glClearBufferfv(GL_DEPTH, 0, &value));
    return *this;
}

ENGINE_EXPORT auto FramebufferDrawCtx::ClearStencil(GLint value) const -> FramebufferDrawCtx const& {
    GLCALL(glClearBufferiv(GL_STENCIL, 0, &value));
    return *this;
}

ENGINE_EXPORT auto FramebufferDrawCtx::ClearDepthStencil(GLfloat depth, GLint stencil) const
    -> FramebufferDrawCtx const& {
    GLCALL(glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil));
    return *this;
}

// auto FramebufferDrawCtx::LinkBackbuffer(GLenum attachment, GLint texLevel) -> FramebufferDrawCtx&& {
//     assert(attachment >= GL_COLOR_ATTACHMENT0 && attachment <= GL_COLOR_ATTACHMENT31);
//     GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, texLevel));
// return *this;
// }

// valid attachments: COLOR_ATTACHMENTi, DEPTH_ATTACHMENT, or STENCIL_ATTACHMENT
ENGINE_EXPORT auto FramebufferDrawCtx::Invalidate(GlContext const& gl, CpuMemory<GLenum const> attachments) const
    -> FramebufferDrawCtx const& {
    if (gl.Extensions().Supports(GlExtensions::ARB_invalidate_subdata)) {
        GLCALL(glInvalidateFramebuffer(framebufferTarget_, attachments.NumElements(), attachments[0]));
    }
    return *this;
}

ENGINE_EXPORT auto FramebufferDrawCtx::IsComplete(GlContext const& gl) const -> bool {
    bool isComplete = false;
    GLCALL(isComplete = glCheckFramebufferStatus(framebufferTarget_) == GL_FRAMEBUFFER_COMPLETE);
    if (!isComplete) {
        LogDebugLabelUnsafe(gl, contextFramebuffer_, GlObjectType::FRAMEBUFFER, "Framebuffer is not complete");
    }
    return isComplete;
}

ENGINE_EXPORT FramebufferEditCtx::FramebufferEditCtx(Framebuffer& useFramebuffer, bool bindAsDraw) noexcept
    : ctx_{useFramebuffer, bindAsDraw}
    , fb_{useFramebuffer} { }

ENGINE_EXPORT auto FramebufferEditCtx::AttachTexture(
    GlContext& gl, GLenum attachment, Texture const& tex, GLint texLevel, GLint arrayIndex) const
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
        int32_t const drawBuffer     = attachment - GL_COLOR_ATTACHMENT0;
        fb_.drawBuffers_[drawBuffer] = attachment;
        assert(IsComplete(gl));
    } else {
        assert(false && "Failed to set framebuffer attachment");
    }
    return *this;
}

ENGINE_EXPORT auto FramebufferEditCtx::AttachRenderbuffer(
    GlContext& gl, GLenum attachment, Renderbuffer const& rb, GLint arrayIndex) const
    -> FramebufferEditCtx const& {
    GLCALL(glFramebufferRenderbuffer(ctx_.BoundTarget(), attachment, rb.RenderbufferSlotTarget(), rb.Id()));
    int32_t const drawBuffer     = attachment - GL_COLOR_ATTACHMENT0;
    fb_.drawBuffers_[drawBuffer] = attachment;
    return *this;
}

ENGINE_EXPORT auto FramebufferEditCtx::CommitDrawbuffers() const -> FramebufferEditCtx const& {
    assert(ctx_.BoundTarget() == GL_DRAW_FRAMEBUFFER);
    GLCALL(glDrawBuffers(static_cast<GLsizei>(Framebuffer::MAX_DRAW_BUFFERS), fb_.drawBuffers_));
    return *this;
}

ENGINE_EXPORT auto FramebufferEditCtx::SetReadbuffer(GLenum attachment) const -> FramebufferEditCtx const& {
    assert(ctx_.BoundTarget() == GL_READ_FRAMEBUFFER);
    GLCALL(glReadBuffer(attachment));
    return *this;
}

} // namespace engine::gl
