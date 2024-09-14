#include "engine/gl/CommonRenderers.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Guard.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/TextureUnits.hpp"
#include "engine/gl/Uniform.hpp"
#include "engine/gl/Vao.hpp"

namespace {

constexpr GLint BLIT_UNIFORM_TEXTURE_LOCATION  = 0;
constexpr GLint BLIT_UNIFORM_UV_SCALE_LOCATION = 1;
constexpr GLint BLIT_TEXTURE_SLOT              = 0; // TODO: 1 and above slots don't work

auto AllocateBlitter() -> engine::gl::GpuProgram {
    using namespace engine;

    gl::shader::Define const defines[] = {
        {.name = "UNIFORM_TEXTURE", .value = BLIT_UNIFORM_TEXTURE_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "UNIFORM_UV_SCALE", .value = BLIT_UNIFORM_UV_SCALE_LOCATION, .type = gl::shader::Define::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/triangle_fullscreen.vert", "data/engine/shaders/blit.frag",
        CpuView{defines, std::size(defines)}, "Blit");
    assert(maybeProgram);
    gl::GpuProgram blitProgram = std::move(*maybeProgram);

    auto programGuard = gl::UniformCtx(blitProgram);
    programGuard.SetUniformTexture(BLIT_UNIFORM_TEXTURE_LOCATION, BLIT_TEXTURE_SLOT);
    programGuard.SetUniformValue2(BLIT_UNIFORM_UV_SCALE_LOCATION, 1.0f, 1.0f);

    return blitProgram;
}

} // namespace

namespace engine::gl {

void CommonRenderers::Initialize() {
    if (isInitialized_) { return; }
    XLOG("CommonRenderers::Initialize", 0);
    axesRenderer_      = AxesRenderer::Allocate();
    boxRenderer_       = BoxRenderer::Allocate();
    frustumRenderer_   = FrustumRenderer::Allocate();
    billboardRenderer_ = BillboardRenderer::Allocate();

    lineRenderer_  = LineRenderer::Allocate(MAX_LINES);
    pointRenderer_ = PointRenderer::Allocate(MAX_POINTS);

    datalessTriangleVao_ = Vao::Allocate("Dataless Triangle VAO");
    (void)VaoMutableCtx{datalessTriangleVao_}.MakeUnindexed(3);

    datalessQuadVao_ = Vao::Allocate("Dataless Quad VAO");
    (void)VaoMutableCtx{datalessQuadVao_}.MakeUnindexed(4);

    isInitialized_ = true;
    blitProgram_   = AllocateBlitter();

    samplerNearest_ = samplersCache_.Store(
        "clamp/nearest",
        gl::GpuSampler::Allocate("Sampler/Nearset")
            .WithLinearMagnify(false)
            .WithLinearMinify(false)
            .WithWrap(GL_CLAMP_TO_EDGE));
    samplerLinear_ = samplersCache_.Store(
        "clamp/linear",
        gl::GpuSampler::Allocate("Sampler/Linear")
            .WithLinearMagnify(true)
            .WithLinearMinify(true)
            .WithWrap(GL_CLAMP_TO_EDGE));
    samplerLinearRepeat_ = samplersCache_.Store(
        "repeat/linear",
        gl::GpuSampler::Allocate("Sampler/LinearRepeat")
            .WithLinearMagnify(true)
            .WithLinearMinify(true)
            .WithWrap(GL_REPEAT));

    stubColorTexture_ = gl::Texture::Allocate2D(GL_TEXTURE_2D, glm::ivec3(1, 1, 0), GL_RGB8, "Stub color");
    constexpr uint8_t TEXTURE_DATA_STUB_COLOR[] = {
        255,
        42,
        255,
    };
    (void)gl::TextureCtx{stubColorTexture_}.Fill2D(
        {.dataFormat = GL_RGB,
         .dataType   = GL_UNSIGNED_BYTE,
         .data       = TEXTURE_DATA_STUB_COLOR,
         .size       = stubColorTexture_.Size()});
}

void CommonRenderers::RenderAxes(glm::mat4 const& mvp, float scale, ColorCode color) {
    assert(IsInitialized() && "Bad call to RenderAxes, CommonRenderers isn't initialized");
    axesRenderer_.Render(mvp, scale);
    debugPoints_.PushPoint(glm::scale(mvp, glm::vec3{scale * 0.1f}), color);
}

void CommonRenderers::RenderAxes(glm::mat4 const& mvp, float scale) {
    assert(IsInitialized() && "Bad call to RenderAxes, CommonRenderers isn't initialized");
    axesRenderer_.Render(mvp, scale);
}

void CommonRenderers::RenderBox(glm::mat4 const& centerMvp, glm::vec4 color) const {
    assert(IsInitialized() && "Bad call to RenderBox, CommonRenderers isn't initialized");
    boxRenderer_.Render(centerMvp, color);
}

void CommonRenderers::RenderFrustum(
    glm::mat4 const& centerMvp, Frustum const& frustum, glm::vec4 color, float thickness) const {
    assert(IsInitialized() && "Bad call to RenderFrustum, CommonRenderers isn't initialized");
    frustumRenderer_.Render(centerMvp, frustum, color, thickness);
}

void CommonRenderers::RenderFulscreenTriangle() const {
    assert(IsInitialized() && "Bad call to RenderFulscreenTriangle, CommonRenderers isn't initialized");
    RenderVao(datalessTriangleVao_);
}

void CommonRenderers::RenderBillboard(BillboardRenderArgs const& args) const {
    assert(IsInitialized() && "Bad call to RenderBillboard, CommonRenderers isn't initialized");
    billboardRenderer_.Render(args);
}

void CommonRenderers::RenderLines(glm::mat4 const& camera) const {
    assert(IsInitialized() && "Bad call to RenderLines, CommonRenderers isn't initialized");
    lineRenderer_.Render(camera);
}

void CommonRenderers::FlushLinesToGpu(std::vector<LineRendererInput::Line> const& lines) {
    assert(IsInitialized() && "Bad call to FlushLinesToGpu, CommonRenderers isn't initialized");
    auto linesOffset = 0;
    {
        linesOffset += debugLines_.DataSize();
        if (debugLines_.IsDataDirty()) {
            lineRenderer_.Fill(debugLines_.Data(), debugLines_.DataSize(), 0);
            debugLines_.Clear();
        }
    }
    {
        lineRenderer_.Fill(lines, std::size(lines), linesOffset);
        linesOffset += std::size(lines);
    }
}

void CommonRenderers::RenderPoints(glm::mat4 const& camera) const {
    assert(IsInitialized() && "Bad call to RenderPoints, CommonRenderers isn't initialized");
    pointRenderer_.Render(glm::mat4{1.0f}, 0, pointsFirstExternalIdx_);
    pointRenderer_.Render(camera, pointsFirstExternalIdx_, pointsLimit_);
}

void CommonRenderers::FlushPointsToGpu(std::vector<PointRendererInput::Point> const& points) {
    assert(IsInitialized() && "Bad call to FlushPointsToGpu, CommonRenderers isn't initialized");
    auto pointsOffset = 0;
    {
        pointsOffset += debugPoints_.DataSize();
        if (debugPoints_.IsDataDirty()) {
            pointRenderer_.Fill(debugPoints_.Data(), debugPoints_.DataSize(), 0);
            debugPoints_.Clear();
        }
    }
    pointsFirstExternalIdx_ = pointsOffset;
    {
        pointRenderer_.Fill(points, std::size(points), pointsOffset);
        pointsOffset += std::size(points);
    }
    pointsLimit_ = pointsOffset;
}

void CommonRenderers::Blit2D(GLuint srcTexture, glm::vec2 uvScale) const {
    assert(IsInitialized() && "Bad call to Blit2D, CommonRenderers isn't initialized");
    auto programGuard = gl::UniformCtx(blitProgram_);
    programGuard.SetUniformValue2(BLIT_UNIFORM_UV_SCALE_LOCATION, uvScale.x, uvScale.y);
    gl::GlTextureUnits::Bind2D(BLIT_TEXTURE_SLOT, srcTexture);
    // auto depthGuard = gl::GlGuardDepth(false);

    GLCALL(glEnable(GL_CULL_FACE));
    GLCALL(glDisable(GL_BLEND));
    GLCALL(glDisable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_FALSE));

    RenderFulscreenTriangle();
}

auto CommonRenderers::CacheSampler(std::string_view name, GpuSampler&& sampler) -> SamplersCache::CacheKey {
    return samplersCache_.Store(name, std::move(sampler));
}

auto CommonRenderers::FindSampler(SamplersCache::CacheKey sampler) const -> GpuSampler const& {
    return samplersCache_.GetSampler(sampler);
}

void CommonRenderers::OnFrameEnd() {
    if (debugPoints_.IsDataDirty()) {
        if (pointsFirstExternalIdx_ > 0) {
            if (debugPoints_.DataSize() > pointsFirstExternalIdx_) {
                XLOGW(
                    "Too many internal points, some will be ignored (actual={}, limit={})", debugPoints_.DataSize(),
                    pointsFirstExternalIdx_);
            }
            pointRenderer_.Fill(debugPoints_.Data(), pointsFirstExternalIdx_, 0);
        } else {
            pointsFirstExternalIdx_ = debugPoints_.DataSize();
            pointRenderer_.Fill(debugPoints_.Data(), pointsFirstExternalIdx_, 0);
            pointsLimit_ = pointsFirstExternalIdx_;
            pointRenderer_.LimitInstances(pointsLimit_);
        }
        debugPoints_.Clear();
    }
    // if (debugLines_.IsDataDirty()) {
    //     lineRenderer_.Fill(debugLines_.Data());
    debugLines_.Clear();
    // }
}

} // namespace engine::gl