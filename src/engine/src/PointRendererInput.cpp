#include "engine/PointRendererInput.hpp"

namespace engine {

PointRendererInput::PointRendererInput(size_t maxPoints) noexcept
    : maxPoints_(maxPoints) {
    points_.reserve(maxPoints_);
    Clear();
}

void PointRendererInput::Clear() {
    points_.clear();
    currentColor_ = ColorCode::WHITE;
    isDirty_      = false;
}

void PointRendererInput::SetColor(ColorCode color) { currentColor_ = color; }

void PointRendererInput::PushPoint(glm::vec3 worldPosition, float scale) {
    PushPoint(worldPosition, scale, currentColor_);
}

void PointRendererInput::PushPoint(glm::vec3 worldPosition, float scale, ColorCode color) {
    PushPoint(glm::scale(glm::translate(glm::mat4{1.0f}, worldPosition), glm::vec3{scale}), color);
}

void PointRendererInput::PushPoint(glm::mat4 const& transformToWorld, ColorCode color) {
    if (std::size(points_) >= maxPoints_) {
        // NOTE: not hard limit, but better be, because it
        // reallocates the lines_ vector
        XLOGW("PointRendererInput too many points are pushed {}", std::size(points_));
    }
    int32_t colorIdx = static_cast<int32_t>(color);
    points_.emplace_back(Point{.transform = transformToWorld, .colorIdx = colorIdx});
    isDirty_ = true;
}

} // namespace engine
