#pragma once

#include "engine/gl/AxesRenderer.hpp"
#include "engine/gl/BoxRenderer.hpp"
#include "engine/gl/FrustumRenderer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Sampler.hpp"
#include "engine/gl/Texture.hpp"

#include <glad/gl.h>
#include <glm/mat4x4.hpp>

namespace engine::gl {

class CommonRenderers final {
public:
#define Self CommonRenderers
    explicit Self()              = default;
    ~Self()                      = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = default;
#undef Self

    void Initialize();
    auto IsInitialized [[nodiscard]] () { return isInitialized_; }

    void RenderAxes(glm::mat4 const& mvp);
    // static void RenderLine(glm::vec3 worldBegin, glm::vec3 worldEnd, glm::mat4 const& viewProjection);
    void RenderBox(glm::mat4 const& centerMvp, glm::vec4 color = glm::vec4{1.0f});
    void RenderFrustum(glm::mat4 const& centerMvp, glm::vec4 color = glm::vec4{1.0f});

    void RenderFulscreenTriangle();
    void Blit2D(GLuint srcTexture);

    auto VaoFullscreen() -> Vao const& { return fullscreenTriangleVao_; }
    auto SamplerNearest() -> Sampler const& { return samplerNearest_; }
    auto SamplerLinear() -> Sampler const& { return samplerLinear_; }
    auto SamplerLinearMips() -> Sampler const& { return samplerLinearMip_; }
    auto TextureStubColor() -> Texture const& { return stubColorTexture_; }

private:
    AxesRenderer axesRenderer_;
    BoxRenderer boxRenderer_;
    FrustumRenderer frustumRenderer_;
    Vao fullscreenTriangleVao_;
    GpuProgram blitProgram_;
    bool isInitialized_;
    Sampler samplerNearest_;
    Sampler samplerLinear_;
    Sampler samplerLinearMip_;
    Texture stubColorTexture_;
};

} // namespace engine::gl
