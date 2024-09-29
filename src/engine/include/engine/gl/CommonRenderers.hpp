#pragma once

#include "engine/gl/AxesRenderer.hpp"
#include "engine/gl/BillboardRenderer.hpp"
#include "engine/gl/BoxRenderer.hpp"
#include "engine/gl/Context.hpp"
#include "engine/gl/FrustumRenderer.hpp"
#include "engine/gl/IGlDisposable.hpp"
#include "engine/gl/LineRenderer.hpp"
#include "engine/gl/PointRenderer.hpp"
#include "engine/gl/GpuProgram.hpp"
#include "engine/gl/SamplersCache.hpp"
#include "engine/gl/Texture.hpp"

#include <glm/mat4x4.hpp>

namespace engine::gl {

class CommonRenderers final : public IGlDisposable {

public:
#define Self CommonRenderers
    explicit Self() noexcept     = default;
    ~Self() override             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    void Dispose(GlContext const& gl) override;

    void Initialize(GlContext& gl);
    auto IsInitialized [[nodiscard]] () const -> bool { return isInitialized_; }
    void OnFrameEnd();

    void RenderAxes(GlContext const& gl, glm::mat4 const& mvp, float scale, ColorCode color);
    void RenderAxes(GlContext const& gl, glm::mat4 const& mvp, float scale = 1.0f);
    // static void RenderLine(glm::vec3 worldBegin, glm::vec3 worldEnd, glm::mat4 const& viewProjection);
    void RenderBox(GlContext const& gl, glm::mat4 const& centerMvp, glm::vec4 color = glm::vec4{1.0f}) const;
    void RenderFrustum(
        GlContext const& gl, glm::mat4 const& centerMvp, Frustum const& frustum, glm::vec4 color = glm::vec4{1.0f},
        float thickness = 0.015f) const;
    void RenderBillboard(GlContext const& gl, BillboardRenderArgs const& args) const;
    void RenderLines(GlContext const& gl, glm::mat4 const& camera) const;
    void FlushLinesToGpu(std::vector<LineRendererInput::Line> const&);
    void RenderPoints(GlContext const& gl, glm::mat4 const& camera) const;
    void FlushPointsToGpu(std::vector<PointRendererInput::Point> const&);

    void RenderFulscreenTriangle(GlContext const& gl) const;
    void Blit2D(GlContext& gl, GLuint srcTexture, glm::vec2 uvScale = glm::vec2{1.0f}) const;

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
    AxesRenderer axesRenderer_ = AxesRenderer{};
    BoxRenderer boxRenderer_ = BoxRenderer{};
    FrustumRenderer frustumRenderer_ = FrustumRenderer{};
    BillboardRenderer billboardRenderer_ = BillboardRenderer{};

    constexpr static size_t MAX_LINES = 10'000;
    LineRenderer lineRenderer_ = LineRenderer{};
    LineRendererInput debugLines_ = LineRendererInput{MAX_LINES};

    constexpr static size_t MAX_POINTS = 10'000;
    PointRenderer pointRenderer_ = PointRenderer{};
    int32_t pointsFirstExternalIdx_ = 0;
    int32_t pointsLimitInternal_ = 0;
    int32_t pointsLimitExternal_ = 0;
    PointRendererInput debugPoints_ = PointRendererInput{MAX_POINTS};

    Vao datalessTriangleVao_ = Vao{};
    Vao datalessQuadVao_ = Vao{};
    GpuProgramHandle blitProgram_ = GpuProgramHandle{};
    SamplersCache::CacheKey samplerNearest_ = {};
    SamplersCache::CacheKey samplerLinear_ = {};
    SamplersCache::CacheKey samplerLinearRepeat_ = {};
    SamplersCache samplersCache_ = SamplersCache{};
    Texture stubColorTexture_ = Texture{};
    bool isInitialized_ = false;

    std::vector<IGlDisposable*> toBeDisposed_ = {};
};

} // namespace engine::gl
