#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

struct Frustum final {
    float left;
    float right;
    float bottom;
    float top;
    float near;
    float far;
    Frustum(float left, float right, float bottom, float top, float near, float far)
        : left(left)
        , right(right)
        , bottom(bottom)
        , top(top)
        , near(near)
        , far(far) { }
};

class FrustumRenderer final {

public:
#define Self FrustumRenderer
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] () -> FrustumRenderer;
    void Render(glm::mat4 const& originMvp, Frustum const& frustum, glm::vec4 color = glm::vec4(1.0)) const;

private:
    Vao vao_{};
    GpuBuffer attributeBuffer_{};
    GpuBuffer indexBuffer_{};
    GpuProgram program_{};
    GpuBuffer ubo_{};
};

} // namespace engine::gl
