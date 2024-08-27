#pragma once

#include "engine/Prelude.hpp"

#include <stack>

namespace engine {

class SphereRendererInput final {

public:
#define Self SphereRendererInput
    explicit Self(size_t maxSpheres = 10'000) noexcept;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    struct Sphere final {
        glm::mat4 transform;
        int32_t colorIdx;
    };

    auto IsDataDirty [[nodiscard]] () -> bool { return isDirty_; }
    auto Data [[nodiscard]] () -> std::vector<Sphere> const& {
        isDirty_ = false;
        return spheres_;
    }
    void SetColor(ColorCode color);
    void PushSphere(glm::vec3 worldPosition, float scale = 1.0f);
    void Clear();

private:
    struct ColorCtx final {
        glm::vec3 color;
        int32_t colorIdx;
    };

    size_t maxSpheres_{};
    std::vector<Sphere> spheres_{};
    std::stack<ColorCtx> colorContexts_{};
    bool isDirty_{false};
};

} // namespace engine