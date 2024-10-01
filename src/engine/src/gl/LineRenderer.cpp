#include "engine/gl/LineRenderer.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

#include "engine_private/Prelude.hpp"

namespace {

constexpr GLint UNIFORM_MVP_LOCATION = 0;

} // namespace

namespace engine::gl {

ENGINE_EXPORT auto LineRenderer::Allocate(GlContext const& gl, size_t maxLines) -> LineRenderer {
    constexpr GLint ATTRIB_POSITION_LOCATION = 0;
    constexpr GLint ATTRIB_COLOR_LOCATION    = 1;

    LineRenderer renderer;
    renderer.attributeBuffer_ = gl::GpuBuffer::Allocate(
        gl, GL_ARRAY_BUFFER, gl::GpuBuffer::CLIENT_UPDATE,
        CpuMemory<void const>{nullptr, maxLines * sizeof(LineRendererInput::Line)}, "LineRenderer Vertices");
    renderer.vao_ = gl::Vao::Allocate(gl, "LineRenderer VAO");
    std::ignore   = gl::VaoMutableCtx{renderer.vao_}
                      .MakeVertexAttribute(
                          renderer.attributeBuffer_,
                          {.location        = ATTRIB_POSITION_LOCATION,
                           .valuesPerVertex = 3,
                           .datatype        = GL_FLOAT,
                           .stride          = sizeof(LineRendererInput::Vertex),
                           .offset          = offsetof(LineRendererInput::Vertex, position)})
                      .MakeVertexAttribute(
                          renderer.attributeBuffer_,
                          {.location        = ATTRIB_COLOR_LOCATION,
                           .valuesPerVertex = 1,
                           .datatype        = GL_UNSIGNED_INT,
                           .stride          = sizeof(LineRendererInput::Vertex),
                           .offset          = offsetof(LineRendererInput::Vertex, colorIdx)})
                      .MakeUnindexed(maxLines * 2);

    std::vector<ShaderDefine> defines = {
        ShaderDefine::I32("ATTRIB_POSITION", ATTRIB_POSITION_LOCATION),
        ShaderDefine::I32("ATTRIB_COLOR", ATTRIB_COLOR_LOCATION),
        ShaderDefine::I32("UNIFORM_MVP", UNIFORM_MVP_LOCATION),
    };

    auto maybeProgram = gl.Programs()->LinkProgramFromFiles(
        gl, "data/engine/shaders/lines.vert", "data/engine/shaders/color_palette.frag", std::move(defines),
        "LineRenderer");
    assert(maybeProgram);
    renderer.program_ = std::move(*maybeProgram);

    return renderer;
}

ENGINE_EXPORT void LineRenderer::Dispose(GlContext const& gl) {
    gl.Programs()->DisposeProgram(std::move(program_));
}

ENGINE_EXPORT void LineRenderer::Render(GlContext const& gl, glm::mat4 const& camera) const {
    auto programGuard = UniformCtx{gl.GetProgram(program_)};
    programGuard.SetUniformMatrix4x4(UNIFORM_MVP_LOCATION, glm::value_ptr(camera));
    RenderVao(vao_, GL_LINES);
}

ENGINE_EXPORT void LineRenderer::Fill(
    std::vector<LineRendererInput::Line> const& lines, size_t numLines, size_t numLinesOffset) const {
    using T               = typename std::decay<decltype(*lines.begin())>::type;
    auto const byteOffset = numLinesOffset * sizeof(T);
    auto const numBytes   = std::min(
        attributeBuffer_.SizeBytes() - byteOffset, // buffer limit
        numLines * sizeof(T)                       // argument limit
    );
    attributeBuffer_.Fill(CpuMemory<GLvoid const>{lines.data(), numBytes}, byteOffset);
}

} // namespace engine::gl
