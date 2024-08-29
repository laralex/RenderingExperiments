#include "engine/LineRendererInput.hpp"

namespace engine {

LineRendererInput::LineRendererInput(size_t maxLines) noexcept
    : maxLines_(maxLines) {
    lines_.reserve(maxLines_);
    Clear();
}

void LineRendererInput::Clear() {
    lines_.clear();
    currentColor_ = ColorCode::WHITE;
    isDirty_      = false;
    hasTransform_ = false;
}

void LineRendererInput::SetTransform(glm::mat4 const& transform) {
    customTransform_ = transform;
    hasTransform_    = true;
}

void LineRendererInput::SetTransform() { hasTransform_ = false; }

void LineRendererInput::SetColor(ColorCode color) { currentColor_ = color; }

void LineRendererInput::PushLine(glm::vec3 worldBegin, glm::vec3 worldEnd) {
    if (std::size(lines_) >= maxLines_) {
        // NOTE: not hard limit, but better be, because it
        // reallocates the lines_ vector
        XLOGW("LineRendererInput too many lines are added {}", std::size(lines_));
    }
    int32_t colorIdx = static_cast<int32_t>(currentColor_);
    if (hasTransform_) {
        glm::vec4 homo = customTransform_ * glm::vec4{worldBegin, 1.0f};
        worldBegin     = glm::vec3{homo} / homo.w;
        homo           = customTransform_ * glm::vec4{worldEnd, 1.0f};
        worldEnd       = glm::vec3{homo} / homo.w;
    }
    lines_.emplace_back(Line{Vertex{worldBegin, colorIdx}, Vertex{worldEnd, colorIdx}});
    isDirty_ = true;
}

void LineRendererInput::PushRay(glm::vec3 worldBegin, glm::vec3 worldDirection) {
    PushLine(worldBegin, worldBegin + worldDirection);
}

} // namespace engine
