#pragma once

#include "engine/Prelude.hpp"

#include <stack>

namespace engine {

class PointRendererInput final {

public:
#define Self PointRendererInput
    explicit Self(size_t maxPoints = 10'000) noexcept;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    struct Point final {
        glm::mat4 transform;
        int32_t colorIdx;
    };

    auto IsDataDirty [[nodiscard]] () const -> bool { return isDirty_; }
    auto DataSize [[nodiscard]] () const -> int32_t { return points_.size(); }
    auto Data [[nodiscard]] () -> std::vector<Point> const& {
        isDirty_ = false;
        return points_;
    }
    void SetColor(ColorCode color);
    void PushPoint(glm::vec3 worldPosition, float scale = 1.0f);
    void PushPoint(glm::vec3 worldPosition, float scale, ColorCode color);
    void PushPoint(glm::mat4 const& transformToWorld, ColorCode color);
    void Clear();

private:
    size_t maxPoints_{};
    std::vector<Point> points_{};
    ColorCode currentColor_{};
    bool isDirty_{false};
};

} // namespace engine