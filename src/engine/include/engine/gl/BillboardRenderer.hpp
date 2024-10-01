#pragma once

#include "engine/Precompiled.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Context.hpp"
#include "engine/gl/IGlDisposable.hpp"
#include "engine/gl/GpuProgram.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

struct BillboardRenderArgs;

class BillboardRenderer final : public IGlDisposable {

public:
#define Self BillboardRenderer
    explicit Self() noexcept     = default;
    ~Self() override             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    // FragmentShader inputs:
    // in vec2 v_Uv;
    // If not given, a plain texture is rendered bound to texture slot
    // BillboardRenderer::DEFAULT_UNIFORM_TEXTURE_LOCATION
    static auto Allocate [[nodiscard]] (GlContext& gl, GLuint fragmentShader = GL_NONE) -> BillboardRenderer;
    void Render(GlContext const& gl, BillboardRenderArgs const& args) const;
    void Dispose(GlContext const& gl) override;

private:
    std::shared_ptr<GpuProgram> customVaoProgram_ = {};
    std::shared_ptr<GpuProgram> quadVaoProgram_ = {};
    GpuBuffer ubo_ = GpuBuffer{};
    GLint uboLocation_ = -1;

    static GLint constexpr DEFAULT_UNIFORM_TEXTURE_LOCATION = 0;
};

struct BillboardRenderArgs final {
    struct ShaderArgs final {
        alignas(16) glm::mat4 pivotMvp{1.0f};
        alignas(16) glm::vec3 localPivotOffset{0.0f};
        alignas(16) glm::vec2 localSizeAndAspect{1.0f};
        ScreenShaderArgs screenArgs;
    };

    // VAO must provide positions/uv
    BillboardRenderArgs(
        Vao const& vao, GLenum primitive, ScreenShaderArgs screen, glm::mat4 pivotMvp,
        glm::vec2 localSize = glm::vec2{1.0f, 1.0f}, glm::vec3 localPivotOffset = glm::vec3{0.0f})
        : shaderArgs({pivotMvp, localPivotOffset, localSize, screen})
        , vao(vao)
        , drawPrimitive(primitive) { }

    ShaderArgs shaderArgs;
    Vao const& vao;
    GLenum drawPrimitive = GL_TRIANGLES;
};

} // namespace engine::gl
