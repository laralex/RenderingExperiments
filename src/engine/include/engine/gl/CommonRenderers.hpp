#pragma once

#include "engine/gl/AxesRenderer.hpp"
#include "engine/gl/BillboardRenderer.hpp"
#include "engine/gl/BoxRenderer.hpp"
#include "engine/gl/FrustumRenderer.hpp"
#include "engine/gl/LineRenderer.hpp"
#include "engine/gl/PointRenderer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/SamplersCache.hpp"
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
    Self& operator=(Self&&)      = delete;
#undef Self

    void Initialize();
    auto IsInitialized [[nodiscard]] () const -> bool { return isInitialized_; }
    void OnFrameEnd();

    void RenderAxes(glm::mat4 const& mvp, float scale, ColorCode color);
    void RenderAxes(glm::mat4 const& mvp, float scale = 1.0f);
    // static void RenderLine(glm::vec3 worldBegin, glm::vec3 worldEnd, glm::mat4 const& viewProjection);
    void RenderBox(glm::mat4 const& centerMvp, glm::vec4 color = glm::vec4{1.0f}) const;
    void RenderFrustum(glm::mat4 const& centerMvp, Frustum const& frustum, glm::vec4 color = glm::vec4{1.0f}) const;
    void RenderBillboard(BillboardRenderArgs const& args) const;
    void RenderLines(glm::mat4 const& camera) const;
    void FlushLinesToGpu(std::vector<LineRendererInput::Line> const&);
    void RenderPoints(glm::mat4 const& camera) const;
    void FlushPointsToGpu(std::vector<PointRendererInput::Point> const&);

    void RenderFulscreenTriangle() const;
    void Blit2D(GLuint srcTexture) const;

    auto VaoDatalessTriangle [[nodiscard]] () const -> Vao const& { return datalessTriangleVao_; }
    auto VaoDatalessQuad [[nodiscard]] () const -> Vao const& { return datalessQuadVao_; }
    auto TextureStubColor [[nodiscard]] () const -> Texture const& { return stubColorTexture_; }
    auto SamplerNearest [[nodiscard]] () const -> GpuSampler const& {
        return samplersCache_.GetSampler(samplerNearest_);
    }
    auto SamplerLinear [[nodiscard]] () const -> GpuSampler const& { return samplersCache_.GetSampler(samplerLinear_); }
    auto SamplerLinearRepeat [[nodiscard]] () const -> GpuSampler const& {
        return samplersCache_.GetSampler(samplerLinearRepeat_);
    }

    auto CacheSampler [[nodiscard]] (std::string_view name, GpuSampler&& sampler) -> SamplersCache::CacheKey;
    auto FindSampler [[nodiscard]] (SamplersCache::CacheKey sampler) const -> GpuSampler const&;

private:
    AxesRenderer axesRenderer_{};
    BoxRenderer boxRenderer_{};
    FrustumRenderer frustumRenderer_{};
    BillboardRenderer billboardRenderer_{};

    LineRenderer lineRenderer_{};
    LineRendererInput debugLines_{};

    PointRenderer pointRenderer_{};
    int32_t pointsFirstExternalIdx_{0};
    PointRendererInput debugPoints_{};

    Vao datalessTriangleVao_{};
    Vao datalessQuadVao_{};
    GpuProgram blitProgram_{};
    SamplersCache::CacheKey samplerNearest_{};
    SamplersCache::CacheKey samplerLinear_{};
    SamplersCache::CacheKey samplerLinearRepeat_{};
    SamplersCache samplersCache_{};
    Texture stubColorTexture_{};
    bool isInitialized_ = false;
};

} // namespace engine::gl
