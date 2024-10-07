
#include "engine/gl/EditorGridRenderer.hpp"

#include "engine/gl/Uniform.hpp"
#include "engine_private/Prelude.hpp"

namespace {

struct UboData final {
    alignas(16) glm::vec4 thickColor;
    alignas(16) glm::vec4 thinColor;
    alignas(16) glm::vec4 cameraWorldPosition;
    alignas(16) glm::mat4 viewProjection;
};

constexpr GLint UBO_BINDING = 4; // global for GL

} // namespace

namespace engine::gl {

ENGINE_EXPORT auto EditorGridRenderer::Allocate(GlContext& gl) -> EditorGridRenderer {
    EditorGridRenderer renderer;

    std::vector<ShaderDefine> defines = {
        ShaderDefine::I32("UBO_BINDING", UBO_BINDING),
    };

    auto maybeProgram = LinkProgramFromFiles(
        gl, "data/engine/shaders/editor_grid.vert", "data/engine/shaders/editor_grid.frag", std::move(defines),
        "Editor grid");
    assert(maybeProgram);
    renderer.program_ = std::move(*maybeProgram);

    renderer.ubo_ = gl::GpuBuffer::Allocate(
        gl, GL_UNIFORM_BUFFER, gl::GpuBuffer::CLIENT_UPDATE, CpuMemory<void const>{nullptr, sizeof(UboData)},
        "EditorGridRenderer UBO");
    renderer.uboLocation_ = UniformCtx::GetUboLocation(*renderer.program_, "Ubo");

    return renderer;
}

ENGINE_EXPORT void EditorGridRenderer::Dispose(GlContext const& gl) { }

ENGINE_EXPORT void EditorGridRenderer::Render(GlContext& gl, EditorGridRenderer::RenderArgs const& args) const {
    UboData data{
        .thickColor = args.thickColor,
        .thinColor = args.thinColor,
        .cameraWorldPosition = glm::vec4(args.cameraWorldPosition, 1.0f),
        .viewProjection = args.viewProjection,
    };

    ubo_.Fill(CpuMemory<GLvoid const>{&data, sizeof(data)});
    GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BINDING, ubo_.Id()));

    auto programGuard = gl::UniformCtx(*program_);
    GLCALL(glEnable(GL_BLEND));
    GLCALL(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO));
    RenderVao(gl.VaoDatalessQuad(), GL_TRIANGLE_STRIP);
}

} // namespace engine::gl