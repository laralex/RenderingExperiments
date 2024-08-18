#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

struct FrustumRenderer final {
    FrustumRenderer() = default;
    Vao vao;
    GpuBuffer attributeBuffer;
    GpuBuffer indexBuffer;
    GpuProgram program;
};

auto AllocateFrustumRenderer [[nodiscard]] () -> FrustumRenderer;

void RenderFrustum(FrustumRenderer const& renderer, glm::mat4 const& originMvp, glm::vec4 color);

} // namespace engine::gl
