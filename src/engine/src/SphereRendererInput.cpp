#include "engine/SphereRendererInput.hpp"

namespace engine {

SphereRendererInput::SphereRendererInput(size_t maxSpheres) noexcept
    : maxSpheres_(maxSpheres) {
    spheres_.reserve(maxSpheres_);
    Clear();
}

void SphereRendererInput::Clear() {
    spheres_.clear();
    std::stack<ColorCtx> emptyStack{{{
        .color    = COLOR_PALETTE[static_cast<size_t>(ColorCode::WHITE)],
        .colorIdx = static_cast<int32_t>(ColorCode::WHITE),
    }}};
    colorContexts_.swap(emptyStack);
    isDirty_ = false;
}

void SphereRendererInput::SetColor(ColorCode color) {
    colorContexts_.push(ColorCtx{
        .color    = COLOR_PALETTE[static_cast<size_t>(color)],
        .colorIdx = static_cast<int32_t>(color),
    });
}

void SphereRendererInput::PushSphere(glm::vec3 worldPosition, float scale) {
    if (std::size(spheres_) >= maxSpheres_) {
        // NOTE: not hard limit, but better be, because it
        // reallocates the lines_ vector
        XLOGW("SphereRendererInput too many spheres are added {}", std::size(spheres_));
    }
    int32_t colorIdx = colorContexts_.top().colorIdx;
    spheres_.emplace_back(Sphere{
        .transform = glm::scale(glm::translate(glm::mat4{1.0f}, worldPosition), glm::vec3{scale}),
        .colorIdx  = colorIdx});
    isDirty_ = true;
}

} // namespace engine
