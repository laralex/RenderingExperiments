#pragma once

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

    auto IsDataDirty [[nodiscard]] () const -> bool { return isDirty_; }
    auto DataSize [[nodiscard]] () const -> int32_t { return lines_.size(); }
    auto Data [[nodiscard]] () -> std::vector<Line> const& {
        isDirty_ = false;
        return lines_;
    }
    void SetTransform(glm::mat4 const& transform);
    void SetTransform();
    void SetColor(ColorCode color);
    void PushLine(glm::vec3 worldBegin, glm::vec3 worldEnd);
    void PushRay(glm::vec3 worldBegin, glm::vec3 worldDirection);
    void Clear();

private:
    size_t maxLines_{};
    std::vector<Line> lines_{};
    glm::mat4 customTransform_{};
    ColorCode currentColor_{ColorCode::WHITE};
    bool isDirty_{false};
    bool hasTransform_{false};
};

} // namespace engine