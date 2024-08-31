
#include "engine/gl/BillboardRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

constexpr GLint UBO_CONTEXT_BINDING = 0; // global for GL

} // namespace

namespace engine::gl {

auto BillboardRenderer::Allocate(GLuint fragmentShader) -> BillboardRenderer {
    constexpr GLint ATTRIB_POSITION_LOCATION     = 0;
    constexpr GLint ATTRIB_INNER_MARKER_LOCATION = 1;
    constexpr GLint UNIFORM_COLOR_LOCATION       = 0;
    BillboardRenderer renderer;

    gl::ShaderDefine const defines[] = {
        {.name = "UBO_BINDING", .value = UBO_CONTEXT_BINDING, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_COLOR_LOCATION", .value = UNIFORM_COLOR_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name  = "UNIFORM_TEXTURE_LOCATION",
         .value = BillboardRenderer::DEFAULT_UNIFORM_TEXTURE_LOCATION,
         .type  = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/billboard_quad.vert", "data/engine/shaders/constant.frag",
        CpuView{defines, std::size(defines)}, "BillboardRenderer - Quad");
    assert(maybeProgram);
    renderer.quadVaoProgram_ = std::move(*maybeProgram);
    auto programGuard        = UniformCtx{renderer.quadVaoProgram_};
    programGuard.SetUniformValue4(UNIFORM_COLOR_LOCATION, 1.0f, 0.42f, 1.0f, 1.0f);
    renderer.uboLocation_ = programGuard.GetUboLocation("Ubo");

    renderer.ubo_ = gl::GpuBuffer::Allocate(
        GL_UNIFORM_BUFFER, GL_STREAM_DRAW, nullptr, sizeof(BillboardRenderArgs::ShaderArgs), "BillboardRenderer UBO");
    // TODO: customVaoProgram_
    // TODO: provided fragment shader
    return renderer;
}

void BillboardRenderer::Render(BillboardRenderArgs const& args) const {
    GpuProgram const& program = quadVaoProgram_;
    auto programGuard         = gl::UniformCtx(program);

    ubo_.Fill(&args.shaderArgs, sizeof(args.shaderArgs));
    GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_CONTEXT_BINDING, ubo_.Id()));
    // programGuard.SetUbo(uboLocation_, UBO_CONTEXT_BINDING);

    GLCALL(glEnable(GL_CULL_FACE));
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_TRUE));
    GLCALL(glDepthFunc(GL_LEQUAL));

    RenderVao(args.vao, args.drawPrimitive);
}

} // namespace engine::gl