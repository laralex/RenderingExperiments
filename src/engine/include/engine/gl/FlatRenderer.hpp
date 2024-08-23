#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

struct FlatRenderer final {
    FlatRenderer() = default;
    GpuProgram program;
    GpuBuffer ubo;
};

auto AllocateFlatRenderer [[nodiscard]] () -> FlatRenderer;

void RenderFlatMesh(
    FlatRenderer const& renderer, Vao const& vaoWithNormal, GLenum primitive, glm::mat4 const& model,
    glm::mat4 const& camera, glm::vec3 lightPosition);

} // namespace engine::gl