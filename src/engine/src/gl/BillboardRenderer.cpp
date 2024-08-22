
#include "engine/gl/BillboardRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

constexpr GLint UBO_CONTEXT_BINDING = 0; // global for GL
constexpr GLint UBO_SHADER_BINDING = 0; // local for shader

} // namespace

namespace engine::gl {

auto AllocateBillboardRenderer(GLuint fragmentShader) -> BillboardRenderer {
    constexpr GLint ATTRIB_POSITION_LOCATION     = 0;
    constexpr GLint ATTRIB_INNER_MARKER_LOCATION = 1;

    BillboardRenderer renderer;

    constexpr static int32_t NUM_DEFINES        = 3;
    gl::ShaderDefine const defines[NUM_DEFINES] = {
        {.name = "UBO_BINDING", .value = UBO_SHADER_BINDING, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_COLOR_LOCATION", .value = 0, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_TEXTURE_LOCATION", .value = BillboardRenderer::DEFAULT_UNIFORM_TEXTURE_LOCATION, .type = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/billboard_quad.vert", "data/engine/shaders/constant.frag", CpuView{defines, NUM_DEFINES},
        "BillboardRenderer - Quad");
    assert(maybeProgram);
    renderer.quadVaoProgram = std::move(*maybeProgram);
    (void)UniformCtx{renderer.quadVaoProgram}
        .SetUniformValue4(0, 1.0f, 0.42f, 1.0f, 1.0f);

    renderer.ubo =
        gl::GpuBuffer::Allocate(GL_UNIFORM_BUFFER, GL_STREAM_DRAW, nullptr, sizeof(BillboardRenderArgs::ShaderArgs), "BillboardRenderer UBO");
    // TODO: customVaoProgram
    // TODO: provided fragment shader
    return renderer;
}

void RenderBillboard(BillboardRenderer const& renderer, BillboardRenderArgs const& args) {
    GpuProgram const& program = renderer.quadVaoProgram;
    auto programGuard = gl::UniformCtx(program);

    renderer.ubo.Fill(&args.shaderArgs, sizeof(args.shaderArgs));
    GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_CONTEXT_BINDING, renderer.ubo.Id()));
    programGuard.SetUbo(UBO_SHADER_BINDING, UBO_CONTEXT_BINDING);

    GLCALL(glEnable(GL_CULL_FACE));
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_TRUE));
    GLCALL(glDepthFunc(GL_LEQUAL));

    RenderVao(args.vao, args.drawPrimitive);
}

} // namespace engine::gl