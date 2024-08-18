#include "engine/gl/CommonRenderers.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Guard.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/TextureUnits.hpp"
#include "engine/gl/Uniform.hpp"
#include "engine/gl/Vao.hpp"

namespace {

constexpr GLint BLIT_UNIFORM_TEXTURE_LOCATION = 0;
constexpr GLint BLIT_TEXTURE_SLOT             = 0; // TODO: 1 and above slots don't work

constexpr uint8_t TEXTURE_DATA_STUB_COLOR[] = {
    255,
    42,
    255,
};

auto AllocateBlitter() -> engine::gl::GpuProgram {
    using namespace engine;
    constexpr static int32_t NUM_FRAG_MACROS     = 1;
    gl::ShaderDefine fragMacros[NUM_FRAG_MACROS] = {
        {.name = "UNIFORM_TEXTURE_LOCATION", .value = BLIT_UNIFORM_TEXTURE_LOCATION, .type = gl::ShaderDefine::INT32},
    };
    std::string vertexShaderCode   = gl::LoadShaderCode("data/engine/shaders/triangle_fullscreen.vert", nullptr, 0);
    std::string fragmentShaderCode = gl::LoadShaderCode("data/engine/shaders/blit.frag", fragMacros, NUM_FRAG_MACROS);
    GLuint vertexShader            = gl::CompileShader(GL_VERTEX_SHADER, vertexShaderCode);
    assert(vertexShader != GL_NONE && "AllocateBlitter");
    GLuint fragmentShader = gl::CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
    assert(fragmentShader != GL_NONE && "AllocateBlitter");

    auto maybeProgram = gl::GpuProgram::Allocate(vertexShader, fragmentShader, "Blit");
    assert(maybeProgram);
    gl::GpuProgram blitProgram = std::move(*maybeProgram);

    GLCALL(glDeleteShader(vertexShader));
    GLCALL(glDeleteShader(fragmentShader));

    auto programGuard = gl::UniformCtx(blitProgram);
    gl::UniformTexture(BLIT_UNIFORM_TEXTURE_LOCATION, BLIT_TEXTURE_SLOT);

    return blitProgram;
}

} // namespace

namespace engine::gl {

void CommonRenderers::Initialize() {
    if (isInitialized_) { return; }

    axesRenderer_          = AllocateAxesRenderer();
    boxRenderer_           = AllocateBoxRenderer();
    frustumRenderer_       = AllocateFrustumRenderer();
    fullscreenTriangleVao_ = Vao::Allocate("Fullscreen triangle");
    isInitialized_         = true;
    blitProgram_           = AllocateBlitter();

    samplerNearest_ = gl::Sampler::Allocate("Sampler nearset")
                          .WithLinearMagnify(false)
                          .WithLinearMinify(false)
                          .WithWrap(GL_CLAMP_TO_EDGE);
    samplerLinear_ = gl::Sampler::Allocate("Sampler linear")
                         .WithLinearMagnify(true)
                         .WithLinearMinify(true)
                         .WithWrap(GL_CLAMP_TO_EDGE);
    samplerLinearMip_ = gl::Sampler::Allocate("Sampler linear+mips")
                            .WithLinearMagnify(true)
                            .WithLinearMinify(true)
                            .WithLinearMinifyOverMips(true, true)
                            .WithAnisotropicFilter(8.0f)
                            .WithWrap(GL_CLAMP_TO_EDGE);
    stubColorTexture_ = gl::Texture::Allocate2D(GL_TEXTURE_2D, glm::ivec3(1, 1, 0), GL_RGB8, "Stub color");
    (void)gl::TextureCtx{stubColorTexture_}.Fill2D(
        GL_RGB, GL_UNSIGNED_BYTE, TEXTURE_DATA_STUB_COLOR, stubColorTexture_.Size());
}

void CommonRenderers::RenderAxes(glm::mat4 const& mvp) {
    assert(IsInitialized() && "Bad call to RenderAxes, CommonRenderers isn't initialized");
    gl::RenderAxes(axesRenderer_, mvp);
}

void CommonRenderers::RenderBox(glm::mat4 const& centerMvp, glm::vec4 color) {
    assert(IsInitialized() && "Bad call to RenderBox, CommonRenderers isn't initialized");
    gl::RenderBox(boxRenderer_, centerMvp, color);
}

void CommonRenderers::RenderFrustum(glm::mat4 const& centerMvp, Frustum const& frustum, glm::vec4 color) {
    assert(IsInitialized() && "Bad call to RenderFrustum, CommonRenderers isn't initialized");
    gl::RenderFrustum(frustumRenderer_, centerMvp, frustum, color);
}

void CommonRenderers::RenderFulscreenTriangle() {
    auto vaoGuard = gl::VaoCtx{fullscreenTriangleVao_};
    GLCALL(glDrawArrays(GL_TRIANGLES, 0, 3));
}

void CommonRenderers::Blit2D(GLuint srcTexture) {
    auto programGuard = gl::UniformCtx(blitProgram_);
    gl::GlTextureUnits::Bind2D(BLIT_TEXTURE_SLOT, srcTexture);
    // auto depthGuard = gl::GlGuardDepth(false);

    GLCALL(glEnable(GL_CULL_FACE));
    GLCALL(glDisable(GL_BLEND));
    GLCALL(glDisable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_FALSE));

    RenderFulscreenTriangle();
}

} // namespace engine::gl