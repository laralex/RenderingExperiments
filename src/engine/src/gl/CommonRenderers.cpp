#include "engine/gl/CommonRenderers.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Guard.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/TextureUnits.hpp"
#include "engine/gl/Uniform.hpp"
#include "engine/gl/Vao.hpp"

namespace {

constexpr GLint BLIT_UNIFORM_TEXTURE_LOCATION = 0;
constexpr GLint BLIT_TEXTURE_SLOT             = 0; // TODO: 1 and above slots don't work

auto AllocateBlitter() -> engine::gl::GpuProgram {
    using namespace engine;

    gl::ShaderDefine const defines[] = {
        {.name = "UNIFORM_TEXTURE_LOCATION", .value = BLIT_UNIFORM_TEXTURE_LOCATION, .type = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/triangle_fullscreen.vert", "data/engine/shaders/blit.frag",
        CpuView{defines, std::size(defines)}, "Blit");
    assert(maybeProgram);
    gl::GpuProgram blitProgram = std::move(*maybeProgram);

    auto programGuard = gl::UniformCtx(blitProgram);
    programGuard.SetUniformTexture(BLIT_UNIFORM_TEXTURE_LOCATION, BLIT_TEXTURE_SLOT);

    return blitProgram;
}

} // namespace

namespace engine::gl {

void CommonRenderers::Initialize() {
    if (isInitialized_) { return; }
    XLOG("CommonRenderers::Initialize", 0);
    axesRenderer_              = AxesRenderer::Allocate();
    boxRenderer_               = BoxRenderer::Allocate();
    frustumRenderer_           = FrustumRenderer::Allocate();
    billboardRenderer_         = BillboardRenderer::Allocate();
    constexpr size_t MAX_LINES = 10'000;
    lineRenderer_              = LineRenderer::Allocate(MAX_LINES);

    datalessTriangleVao_ = Vao::Allocate("Dataless Triangle VAO");
    (void)VaoMutableCtx{datalessTriangleVao_}.MakeUnindexed(3);

    datalessQuadVao_ = Vao::Allocate("Dataless Quad VAO");
    (void)VaoMutableCtx{datalessQuadVao_}.MakeUnindexed(4);

    isInitialized_ = true;
    blitProgram_   = AllocateBlitter();

    samplerNearest_ = samplersCache_.Cache(
        "clamp/nearest",
        gl::Sampler::Allocate("Sampler/Nearset")
            .WithLinearMagnify(false)
            .WithLinearMinify(false)
            .WithWrap(GL_CLAMP_TO_EDGE));
    samplerLinear_ = samplersCache_.Cache(
        "clamp/linear",
        gl::Sampler::Allocate("Sampler/Linear")
            .WithLinearMagnify(true)
            .WithLinearMinify(true)
            .WithWrap(GL_CLAMP_TO_EDGE));
    samplerLinearRepeat_ = samplersCache_.Cache(
        "repeat/linear",
        gl::Sampler::Allocate("Sampler/LinearRepeat")
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
        GL_RGB, GL_UNSIGNED_BYTE, TEXTURE_DATA_STUB_COLOR, stubColorTexture_.Size());
}

void CommonRenderers::RenderAxes(glm::mat4 const& mvp, float scale) const {
    assert(IsInitialized() && "Bad call to RenderAxes, CommonRenderers isn't initialized");
    axesRenderer_.Render(mvp, scale);
}

void CommonRenderers::RenderBox(glm::mat4 const& centerMvp, glm::vec4 color) const {
    assert(IsInitialized() && "Bad call to RenderBox, CommonRenderers isn't initialized");
    boxRenderer_.Render(centerMvp, color);
}

void CommonRenderers::RenderFrustum(glm::mat4 const& centerMvp, Frustum const& frustum, glm::vec4 color) const {
    assert(IsInitialized() && "Bad call to RenderFrustum, CommonRenderers isn't initialized");
    frustumRenderer_.Render(centerMvp, frustum, color);
}

void CommonRenderers::RenderFulscreenTriangle() const {
    assert(IsInitialized() && "Bad call to RenderFulscreenTriangle, CommonRenderers isn't initialized");
    RenderVao(datalessTriangleVao_);
}

void CommonRenderers::RenderBillboard(BillboardRenderArgs const& args) const {
    assert(IsInitialized() && "Bad call to RenderBillboard, CommonRenderers isn't initialized");
    billboardRenderer_.Render(args);
}

void CommonRenderers::RenderLines(glm::mat4 const& camera, std::vector<LineRendererInput::Line> const& lines) const {
    assert(IsInitialized() && "Bad call to RenderLines, CommonRenderers isn't initialized");
    lineRenderer_.Render(camera, lines);
}

void CommonRenderers::Blit2D(GLuint srcTexture) const {
    assert(IsInitialized() && "Bad call to Blit2D, CommonRenderers isn't initialized");
    auto programGuard = gl::UniformCtx(blitProgram_);
    gl::GlTextureUnits::Bind2D(BLIT_TEXTURE_SLOT, srcTexture);
    // auto depthGuard = gl::GlGuardDepth(false);

    GLCALL(glEnable(GL_CULL_FACE));
    GLCALL(glDisable(GL_BLEND));
    GLCALL(glDisable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_FALSE));

    RenderFulscreenTriangle();
}

auto CommonRenderers::CacheSampler(std::string_view name, Sampler&& sampler) -> SamplersCache::CacheKey {
    return samplersCache_.Cache(name, std::move(sampler));
}

auto CommonRenderers::FindSampler(SamplersCache::CacheKey sampler) const -> Sampler const& {
    return samplersCache_.GetSampler(sampler);
}

} // namespace engine::gl