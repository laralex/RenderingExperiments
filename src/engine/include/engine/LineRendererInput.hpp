#pragma once

#include "engine/LineRendererInput.hpp"
#include "engine/Prelude.hpp"

#include <stack>

namespace engine {

class LineRendererInput final {

public:
#define Self LineRendererInput
    explicit Self(size_t maxLines = 10'000) noexcept;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    struct Vertex final {
        glm::vec3 position;
        int32_t colorIdx;
    };

    struct Line final {
        Vertex begin;
        Vertex end;
    };

    auto Data [[nodiscard]] () const -> std::vector<Line> const& { return lines_; }
    void SetColor(ColorCode color);
    void PushLine(glm::vec3 worldBegin, glm::vec3 worldEnd);
    void PushRay(glm::vec3 worldBegin, glm::vec3 worldDirection);
    void Clear();

private:
    struct ColorCtx final {
        glm::vec3 color;
        int32_t colorIdx;
        int32_t oldColorLastLineIdx;
    };

    size_t maxLines_{};
    std::vector<Line> lines_{};
    std::stack<ColorCtx> colorContexts_{};
};

} // namespace engine