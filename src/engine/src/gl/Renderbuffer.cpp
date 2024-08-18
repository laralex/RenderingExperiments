#include "engine/gl/Renderbuffer.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

bool RenderbufferCtx::hasInstances_{false};
GlHandle RenderbufferCtx::contextRenderbuffer_{GL_NONE};
GLenum RenderbufferCtx::contextTarget_{GL_NONE};

RenderbufferCtx::RenderbufferCtx(Renderbuffer const& useRenderbuffer) {
    assert(!hasInstances_);
    contextRenderbuffer_.id  = useRenderbuffer.Id();
    contextTarget_ = useRenderbuffer.RenderbufferSlotTarget();
    GLCALL(glBindRenderbuffer(contextTarget_, contextRenderbuffer_.id));
    hasInstances_ = true;
}

RenderbufferCtx::~RenderbufferCtx() {
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    contextRenderbuffer_.id = GL_NONE;
    GLCALL(glBindRenderbuffer(contextTarget_, contextRenderbuffer_.id));
    hasInstances_ = false;
}

void Renderbuffer::Dispose() {
    if (renderbufferId_ == GL_NONE) { return; }
    LogDebugLabel(*this, "Renderbuffer object was disposed");
    GLCALL(glDeleteRenderbuffers(1, &renderbufferId_.id));
    renderbufferId_.id = GL_NONE;
}

auto Renderbuffer::Allocate2D(glm::ivec2 size, GLenum internalFormat, int32_t msaaSamples, std::string_view name) -> Renderbuffer {

    Renderbuffer renderbuffer{};
    GLCALL(glGenRenderbuffers(1, &renderbuffer.renderbufferId_.id));
    renderbuffer.target_    = GL_RENDERBUFFER;
    renderbuffer.internalFormat_ = internalFormat;
    renderbuffer.size_           = glm::ivec3(size.x, size.y, 0);
    renderbuffer.msaaSamples_    = msaaSamples;

    GLCALL(glBindRenderbuffer(renderbuffer.target_, renderbuffer.renderbufferId_.id));

    // storage requirements can't change in the future
    GLCALL(glRenderbufferStorageMultisample(
        renderbuffer.target_, renderbuffer.msaaSamples_, renderbuffer.internalFormat_, renderbuffer.size_.x, renderbuffer.size_.y));

    if (!name.empty()) {
        DebugLabel(renderbuffer, name);
        LogDebugLabel(renderbuffer, "Renderbuffer was allocated");
    }
    return renderbuffer;
}

auto Renderbuffer::MsaaSamples() const -> std::optional<int32_t> {
    return msaaSamples_ > 0 ? std::optional{msaaSamples_} : std::nullopt;
}

} // namespace engine::gl