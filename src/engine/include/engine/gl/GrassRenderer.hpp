#pragma once

#include "engine/Precompiled.hpp"
#include "engine/gl/GpuBuffer.hpp"
#include "engine/gl/Context.hpp"
#include "engine/gl/IGlDisposable.hpp"
#include "engine/gl/GpuProgram.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

struct GrassRenderArgs;

class GrassRenderer final : public IGlDisposable {

public:
#define Self GrassRenderer
    explicit Self() noexcept     = default;
    ~Self() override             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    // FragmentShader inputs:
    // in vec2 v_Uv;
    static auto Allocate [[nodiscard]] (GlContext& gl, GLuint fragmentShader = GL_NONE) -> GrassRenderer;
    void Render(GlContext& gl, GrassRenderArgs const& args) const;
    void Dispose(GlContext const& gl) override;

private:
    std::shared_ptr<GpuProgram> customVaoProgram_ = {};
    std::shared_ptr<GpuProgram> quadVaoProgram_ = {};
    GpuBuffer ubo_ = GpuBuffer{};

    static GLint constexpr DEFAULT_UNIFORM_TEXTURE_LOCATION = 0;
};

struct GrassRenderArgs final {
    struct alignas(16) Pack final {
        glm::vec3 localPivotOffset;
        float widthDivHeight;
    };
    struct ShaderArgs final {
        alignas(16) glm::mat4 pivotMvp{1.0f};
        alignas(16) Pack pack = {};
        alignas(16) glm::vec2 localSize{1.0f};
    };

    // VAO must provide positions/uv
    GrassRenderArgs(
        Vao const& vao, GLenum primitive, bool isCustomVao, float screenWidthDivHeight, glm::mat4 pivotMvp,
        glm::vec2 localSize = glm::vec2{1.0f, 1.0f}, glm::vec3 localPivotOffset = glm::vec3{0.0f})
        : shaderArgs({pivotMvp, {localPivotOffset, screenWidthDivHeight}, localSize})
        , isCustomVao(isCustomVao)
        , vao(vao)
        , drawPrimitive(primitive) { }

    ShaderArgs shaderArgs = {};
    bool isCustomVao = false;
    Vao const& vao;
    GLenum drawPrimitive = GL_TRIANGLES;
};

} // namespace engine::gl
