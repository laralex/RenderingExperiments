#pragma once

#include "engine/Prelude.hpp"

#include <glm/mat4x4.hpp>

namespace engine::gl {

class AxesRenderer;
class BoxRenderer;
class Vao;
class GpuProgram;
class Sampler;
class Texture;

class CommonRenderers final {
public:
#define Self CommonRenderers
    explicit Self() = delete;
    ~Self() = delete;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    static void Initialize();
    static auto IsInitialized [[nodiscard]] () { return isInitialized_; }

    static void RenderAxes(glm::mat4 const& mvp);
    // static void RenderLine(glm::vec3 worldBegin, glm::vec3 worldEnd, glm::mat4 const& viewProjection);
    static void RenderBox(glm::mat4 const& centerMvp, glm::vec4 color = glm::vec4{1.0f});

    static void RenderFulscreenTriangle();
    static void Blit2D(GLuint srcTexture);

    static auto VaoFullscreen() -> gl::Vao const& { return fullscreenTriangleVao_; }
    static auto SamplerNearest() -> gl::Sampler const& { return samplerNearest_; }
    static auto SamplerLinear() -> gl::Sampler const& { return samplerLinear_; }
    static auto SamplerLinearMips() -> gl::Sampler const& { return samplerLinearMip_; }
    static auto TextureStubColor() -> gl::Texture const& { return stubColorTexture_; }

private:
    static AxesRenderer axesRenderer_;
    static BoxRenderer boxRenderer_;
    static Vao fullscreenTriangleVao_;
    static GpuProgram blitProgram_;
    static bool isInitialized_;
    static Sampler samplerNearest_;
    static Sampler samplerLinear_;
    static Sampler samplerLinearMip_;
    static Texture stubColorTexture_;
};

} // namespace engine::gl
