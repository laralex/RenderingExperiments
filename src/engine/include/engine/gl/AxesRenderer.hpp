#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

struct AxesRenderer final {
    AxesRenderer() = default;
    Vao vao;
    GpuBuffer attributeBuffer;
    GpuBuffer indexBuffer;
    GpuProgram program;
};

auto AllocateAxesRenderer [[nodiscard]] () -> AxesRenderer;

void RenderAxes(AxesRenderer const& renderer, glm::mat4 const& mvp);

} // namespace engine::gl
