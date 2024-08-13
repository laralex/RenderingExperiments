#pragma once

#include "engine/Prelude.hpp"

#include <glm/mat4x4.hpp>

namespace engine::gl {

class AxesRenderer;
class Vao;
class GpuProgram;
class Sampler;

class CommonRenderers final {
public:
    static void Initialize();
    static auto IsInitialized [[nodiscard]] () { return isInitialized_; }
    static void RenderAxes(glm::mat4 const& mvp);
    static void Blit2D(GLuint srcTexture, GLuint dstFramebuffer);
    static auto GetFullscreenTriangle() -> gl::Vao const& { return fullscreenTriangleVao_; }
    static auto GetSamplerNearest() -> gl::Sampler const& { return samplerNearest_; }
    static auto GetSamplerLinear() -> gl::Sampler const& { return samplerLinear_; }
    static auto GetSamplerLinearMips() -> gl::Sampler const& { return samplerLinearMip_; }

private:
    static AxesRenderer axesRenderer_;
    static Vao fullscreenTriangleVao_;
    static GpuProgram blitProgram_;
    static bool isInitialized_;
    static Sampler samplerNearest_;
    static Sampler samplerLinear_;
    static Sampler samplerLinearMip_;
};

} // namespace engine::gl
