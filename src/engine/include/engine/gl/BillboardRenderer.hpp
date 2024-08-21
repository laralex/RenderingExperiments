#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

struct BillboardRenderer final {
    BillboardRenderer() = default;
    GpuProgram customVaoProgram{};
    GpuProgram quadVaoProgram{};
    GpuBuffer ubo{};

    static GLint constexpr DEFAULT_UNIFORM_TEXTURE_LOCATION = 0;
};

// FragmentShader inputs:
// in vec2 v_Uv;
// If not given, a plain texture is rendered bound to texture slot
// BillboardRenderer::DEFAULT_UNIFORM_TEXTURE_LOCATION
auto AllocateBillboardRenderer [[nodiscard]] (GLuint fragmentShader = GL_NONE) -> BillboardRenderer;

// VAO must provide positions/uv
struct BillboardRenderArgs final {
    struct ShaderArgs {
        glm::mat4 pivotMvp{1.0};
        glm::vec3 localPivotOffset{};
        glm::vec2 localSize{};
    };

    BillboardRenderArgs(Vao const& vao, glm::mat4 pivotMvp, glm::vec3 localPivotOffset, glm::vec2 localSize)
        : vao(vao), shaderArgs({pivotMvp, localPivotOffset, localSize}) { }

    Vao const& vao;
    ShaderArgs shaderArgs;
};

void RenderBillboard(BillboardRenderer const& renderer, BillboardRenderArgs const& args);

} // namespace engine::gl
