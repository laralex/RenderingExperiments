#include "engine/gl/Renderbuffer.hpp"

#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_STATIC bool RenderbufferCtx::hasInstances_{false};
ENGINE_STATIC GlHandle RenderbufferCtx::contextRenderbuffer_{GL_NONE};
ENGINE_STATIC GLenum RenderbufferCtx::contextTarget_{GL_NONE};

ENGINE_EXPORT RenderbufferCtx::RenderbufferCtx(Renderbuffer const& useRenderbuffer) noexcept {
    assert(!hasInstances_);
    contextRenderbuffer_.UnsafeAssign(useRenderbuffer.renderbufferId_);
    contextTarget_ = useRenderbuffer.RenderbufferSlotTarget();
    GLCALL(glBindRenderbuffer(contextTarget_, contextRenderbuffer_));
    hasInstances_ = true;
}

ENGINE_EXPORT RenderbufferCtx::~RenderbufferCtx() noexcept {
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    contextRenderbuffer_.UnsafeReset();
    GLCALL(glBindRenderbuffer(contextTarget_, contextRenderbuffer_));
    hasInstances_ = false;
}

ENGINE_EXPORT void Renderbuffer::Dispose() {
    if (renderbufferId_ == GL_NONE) { return; }
    // LogDebugLabel(*this, "Renderbuffer object was disposed");
    XLOG("Renderbuffer object was disposed");
    GLCALL(glDeleteRenderbuffers(1, renderbufferId_.Ptr()));
    renderbufferId_.UnsafeReset();
}

ENGINE_EXPORT auto Renderbuffer::Allocate2D(
    GlContext const& gl, glm::ivec2 size, GLenum internalFormat, int32_t msaaSamples, std::string_view name)
    -> Renderbuffer {

    Renderbuffer renderbuffer{};
    GLCALL(glGenRenderbuffers(1, renderbuffer.renderbufferId_.Ptr()));
    renderbuffer.target_         = GL_RENDERBUFFER;
    renderbuffer.internalFormat_ = internalFormat;
    renderbuffer.size_           = glm::ivec3(size.x, size.y, 0);
    renderbuffer.msaaSamples_    = msaaSamples;

    GLCALL(glBindRenderbuffer(renderbuffer.target_, renderbuffer.renderbufferId_));

    // storage requirements can't change in the future
    GLCALL(glRenderbufferStorageMultisample(
        renderbuffer.target_, renderbuffer.msaaSamples_, renderbuffer.internalFormat_, renderbuffer.size_.x,
        renderbuffer.size_.y));

    if (!name.empty()) {
        DebugLabel(gl, renderbuffer, name);
        LogDebugLabel(gl, renderbuffer, "Renderbuffer was allocated");
    }
    return renderbuffer;
}

ENGINE_EXPORT auto Renderbuffer::MsaaSamples() const -> std::optional<int32_t> {
    return msaaSamples_ > 0 ? std::optional{msaaSamples_} : std::nullopt;
}

} // namespace engine::gl