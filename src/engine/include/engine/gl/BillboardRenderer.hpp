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
    struct ShaderArgs final {
        alignas(16) glm::mat4 pivotMvp{1.0f};
        alignas(16) glm::vec3 localPivotOffset{0.0f};
        alignas(16) glm::vec2 localSizeAndAspect{1.0f};
        ScreenShaderArgs screenArgs;
    };

    BillboardRenderArgs(Vao const& vao, GLenum primitive, ScreenShaderArgs screen, glm::mat4 pivotMvp, glm::vec2 localSize = glm::vec2{1.0f,1.0f}, glm::vec3 localPivotOffset = glm::vec3{0.0f})
        : shaderArgs({pivotMvp, localPivotOffset, localSize, screen}), vao(vao), drawPrimitive(primitive) { }

    ShaderArgs shaderArgs;
    Vao const& vao;
    GLenum drawPrimitive = GL_TRIANGLES;
};

void RenderBillboard(BillboardRenderer const& renderer, BillboardRenderArgs const& args);

} // namespace engine::gl
