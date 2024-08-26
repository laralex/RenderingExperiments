#include "engine/LineRendererInput.hpp"

namespace engine {

LineRendererInput::LineRendererInput(size_t maxLines) noexcept
    : maxLines_(maxLines) {
    lines_.reserve(maxLines_);
    Clear();
}

void LineRendererInput::Clear() {
    colorContexts_.push({
        .color               = {1.0f, 1.0f, 1.0f},
        .colorIdx            = static_cast<int32_t>(ColorCode::WHITE),
        .oldColorLastLineIdx = -1,
    });
    lines_.clear();
    std::stack<ColorCtx> emptyStack{};
    colorContexts_.swap(emptyStack);
}

void LineRendererInput::SetColor(ColorCode color) {
    colorContexts_.push(ColorCtx{
        .color               = COLOR_PALETTE[static_cast<size_t>(color)],
        .colorIdx            = static_cast<int32_t>(color),
        .oldColorLastLineIdx = static_cast<int32_t>(colorContexts_.size()) - 1,
    });
}

void LineRendererInput::PushLine(glm::vec3 worldBegin, glm::vec3 worldEnd) {
    if (std::size(lines_) >= maxLines_) {
        // NOTE: not hard limit, but better be, because it
        // reallocates the lines_ vector
        XLOGW("LineRenderer too many lines are rendered {}", std::size(lines_));
    }
    int32_t colorIdx = colorContexts_.top().colorIdx;
    lines_.emplace_back(Line{Vertex{worldBegin, colorIdx}, Vertex{worldEnd, colorIdx}});
}

void LineRendererInput::PushRay(glm::vec3 worldBegin, glm::vec3 worldDirection) {
    PushLine(worldBegin, worldBegin + worldDirection);
}

} // namespace engine
