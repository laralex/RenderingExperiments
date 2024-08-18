#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

struct PyramidRenderer final {
    PyramidRenderer() = default;
    Vao vao;
    GpuBuffer attributeBuffer;
    GpuBuffer indexBuffer;
    GpuProgram program;
};

auto AllocatePyramidRenderer [[nodiscard]] () -> PyramidRenderer;

void RenderBox(BoxRenderer const& renderer, glm::mat4 const& centerMvp, glm::vec4 color);

} // namespace engine::gl
