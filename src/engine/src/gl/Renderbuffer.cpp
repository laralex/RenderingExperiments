#include "engine/gl/Renderbuffer.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

bool RenderbufferCtx::hasInstances_{false};
GlHandle RenderbufferCtx::contextRenderbuffer_{GL_NONE};
GLenum RenderbufferCtx::contextTarget_{GL_NONE};

RenderbufferCtx::RenderbufferCtx(Renderbuffer const& useRenderbuffer) noexcept {
    assert(!hasInstances_);
    contextRenderbuffer_.UnsafeAssign(useRenderbuffer.renderbufferId_);
    contextTarget_ = useRenderbuffer.RenderbufferSlotTarget();
    GLCALL(glBindRenderbuffer(contextTarget_, contextRenderbuffer_));
    hasInstances_ = true;
}

RenderbufferCtx::~RenderbufferCtx() noexcept {
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    contextRenderbuffer_.UnsafeReset();
    GLCALL(glBindRenderbuffer(contextTarget_, contextRenderbuffer_));
    hasInstances_ = false;
}

void Renderbuffer::Dispose() {
    if (renderbufferId_ == GL_NONE) { return; }
    // LogDebugLabel(*this, "Renderbuffer object was disposed");
    XLOG("Renderbuffer object was disposed", 0);
    GLCALL(glDeleteRenderbuffers(1, &renderbufferId_));
    renderbufferId_.UnsafeReset();
}

auto Renderbuffer::Allocate2D(GlContext const& gl, glm::ivec2 size, GLenum internalFormat, int32_t msaaSamples, std::string_view name)
    -> Renderbuffer {

    Renderbuffer renderbuffer{};
    GLCALL(glGenRenderbuffers(1, &renderbuffer.renderbufferId_));
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

auto Renderbuffer::MsaaSamples() const -> std::optional<int32_t> {
    return msaaSamples_ > 0 ? std::optional{msaaSamples_} : std::nullopt;
}

} // namespace engine::gl