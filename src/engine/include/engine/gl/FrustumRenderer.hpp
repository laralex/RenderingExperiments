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

struct FrustumRenderer final {
    FrustumRenderer() = default;
    Vao vao;
    GpuBuffer attributeBuffer;
    GpuBuffer indexBuffer;
    GpuProgram program;
    GpuBuffer ubo{};
};

auto AllocateFrustumRenderer [[nodiscard]] () -> FrustumRenderer;

void RenderFrustum(
    FrustumRenderer const& renderer, glm::mat4 const& originMvp, Frustum const& frustum,
    glm::vec4 color = glm::vec4(1.0));

} // namespace engine::gl
