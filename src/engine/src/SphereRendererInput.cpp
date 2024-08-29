#include "engine/SphereRendererInput.hpp"

namespace engine {

SphereRendererInput::SphereRendererInput(size_t maxSpheres) noexcept
    : maxSpheres_(maxSpheres) {
    spheres_.reserve(maxSpheres_);
    Clear();
}

void SphereRendererInput::Clear() {
    spheres_.clear();
    currentColor_ = ColorCode::WHITE;
    isDirty_      = false;
}

void SphereRendererInput::SetColor(ColorCode color) { currentColor_ = color; }

void SphereRendererInput::PushSphere(glm::vec3 worldPosition, float scale) {
    PushSphere(worldPosition, scale, currentColor_);
}

void SphereRendererInput::PushSphere(glm::vec3 worldPosition, float scale, ColorCode color) {
    PushSphere(glm::scale(glm::translate(glm::mat4{1.0f}, worldPosition), glm::vec3{scale}), color);
}

void SphereRendererInput::PushSphere(glm::mat4 const& transformToWorld, ColorCode color) {
    if (std::size(spheres_) >= maxSpheres_) {
        // NOTE: not hard limit, but better be, because it
        // reallocates the lines_ vector
        XLOGW("SphereRendererInput too many spheres are pushed {}", std::size(spheres_));
    }
    int32_t colorIdx = static_cast<int32_t>(color);
    spheres_.emplace_back(Sphere{.transform = transformToWorld, .colorIdx = colorIdx});
    isDirty_ = true;
}

} // namespace engine
