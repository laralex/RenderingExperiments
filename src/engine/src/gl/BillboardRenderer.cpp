
#include "engine/gl/BillboardRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

#include "engine_private/Prelude.hpp"

namespace {

constexpr GLint UBO_CONTEXT_BINDING = 0; // global for GL

} // namespace

namespace engine::gl {

ENGINE_EXPORT auto BillboardRenderer::Allocate(GlContext& gl, GLuint fragmentShader) -> BillboardRenderer {
    constexpr GLint ATTRIB_POSITION_LOCATION     = 0;
    constexpr GLint ATTRIB_INNER_MARKER_LOCATION = 1;
    constexpr GLint UNIFORM_COLOR_LOCATION       = 0;
    BillboardRenderer renderer;

    std::vector<ShaderDefine> defines = {
        ShaderDefine::I32("UBO_BINDING", UBO_CONTEXT_BINDING),
        ShaderDefine::I32("UNIFORM_TEXTURE_LOCATION", DEFAULT_UNIFORM_TEXTURE_LOCATION),
    };

    auto maybeProgram = LinkProgramFromFiles(
        gl, "data/engine/shaders/billboard_quad.vert", "data/engine/shaders/uv.frag", std::move(defines),
        "BillboardRenderer - Quad");
    assert(maybeProgram);
    renderer.quadVaoProgram_ = std::move(*maybeProgram);
    auto programGuard        = UniformCtx{*renderer.quadVaoProgram_};
    // programGuard.SetUniformValue4(UNIFORM_COLOR_LOCATION, 1.0f, 0.42f, 1.0f, 1.0f);
    renderer.uboLocation_ = programGuard.GetUboLocation("Ubo");

    renderer.ubo_ = gl::GpuBuffer::Allocate(
        gl, GL_UNIFORM_BUFFER, gl::GpuBuffer::CLIENT_UPDATE,
        CpuMemory<const void>{nullptr, sizeof(BillboardRenderArgs::ShaderArgs)}, "BillboardRenderer UBO");
    // TODO: customVaoProgram_
    // TODO: provided fragment shader
    return renderer;
}

ENGINE_EXPORT void BillboardRenderer::Dispose(GlContext const& gl) { }

ENGINE_EXPORT void BillboardRenderer::Render(GlContext const& gl, BillboardRenderArgs const& args) const {
    auto programGuard         = gl::UniformCtx(*quadVaoProgram_);

    ubo_.Fill(CpuMemory<GLvoid const>{&args.shaderArgs, sizeof(args.shaderArgs)});
    GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_CONTEXT_BINDING, ubo_.Id()));
    // programGuard.SetUbo(uboLocation_, UBO_CONTEXT_BINDING);

    GLCALL(glEnable(GL_CULL_FACE));
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_TRUE));
    GLCALL(glDepthFunc(GL_LEQUAL));

    RenderVao(args.vao, args.drawPrimitive);
}

} // namespace engine::gl