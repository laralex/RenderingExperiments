#pragma once

#include "engine/gl/AxesRenderer.hpp"
#include "engine/gl/BillboardRenderer.hpp"
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
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = default;
#undef Self

    void Initialize();
    auto IsInitialized [[nodiscard]] () const -> bool { return isInitialized_; }

    void RenderAxes(glm::mat4 const& mvp, float scale = 1.0f) const;
    // static void RenderLine(glm::vec3 worldBegin, glm::vec3 worldEnd, glm::mat4 const& viewProjection);
    void RenderBox(glm::mat4 const& centerMvp, glm::vec4 color = glm::vec4{1.0f}) const;
    void RenderFrustum(glm::mat4 const& centerMvp, Frustum const& frustum, glm::vec4 color = glm::vec4{1.0f}) const;
    void RenderBillboard(BillboardRenderArgs const& args) const;

    void RenderFulscreenTriangle() const;
    void Blit2D(GLuint srcTexture) const;

    auto VaoDatalessTriangle [[nodiscard]] () const -> Vao const& { return datalessTriangleVao_; }
    auto VaoDatalessQuad [[nodiscard]] () const -> Vao const& { return datalessQuadVao_; }
    auto SamplerNearest [[nodiscard]] () const -> Sampler const& { return samplerNearest_; }
    auto SamplerLinear [[nodiscard]] () const -> Sampler const& { return samplerLinear_; }
    auto SamplerLinearMips [[nodiscard]] () const -> Sampler const& { return samplerLinearMip_; }
    auto TextureStubColor [[nodiscard]] () const -> Texture const& { return stubColorTexture_; }

private:
    AxesRenderer axesRenderer_;
    BoxRenderer boxRenderer_;
    FrustumRenderer frustumRenderer_;
    BillboardRenderer billboardRenderer_;
    Vao datalessTriangleVao_;
    Vao datalessQuadVao_;
    GpuProgram blitProgram_;
    bool isInitialized_;
    Sampler samplerNearest_;
    Sampler samplerLinear_;
    Sampler samplerLinearMip_;
    Texture stubColorTexture_;
};

} // namespace engine::gl
