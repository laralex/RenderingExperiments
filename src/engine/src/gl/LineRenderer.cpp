#include "engine/gl/LineRenderer.hpp"

#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

constexpr GLint UNIFORM_MVP_LOCATION = 0;

} // namespace

namespace engine::gl {

auto LineRenderer::Allocate(size_t maxLines) -> LineRenderer {
    constexpr GLint ATTRIB_POSITION_LOCATION = 0;
    constexpr GLint ATTRIB_COLOR_LOCATION    = 1;

    LineRenderer renderer;
    renderer.attributeBuffer_ = gl::GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, nullptr, maxLines * sizeof(LineRendererInput::Line), "LineRenderer Vertices");
    renderer.vao_ = gl::Vao::Allocate("LineRenderer VAO");
    (void)gl::VaoMutableCtx{renderer.vao_}
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

    gl::ShaderDefine const defines[] = {
        {.name = "ATTRIB_POSITION", .value = ATTRIB_POSITION_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_COLOR", .value = ATTRIB_COLOR_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_MVP", .value = UNIFORM_MVP_LOCATION, .type = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/lines.vert", "data/engine/shaders/color_palette.frag",
        CpuView{defines, std::size(defines)}, "LineRenderer");
    assert(maybeProgram);
    renderer.program_ = std::move(*maybeProgram);

    return renderer;
}

void LineRenderer::Render(glm::mat4 const& camera) const {
    auto programGuard = UniformCtx{program_};
    programGuard.SetUniformMatrix4(UNIFORM_MVP_LOCATION, glm::value_ptr(camera));
    RenderVao(vao_, GL_LINES);
}

void LineRenderer::Fill(std::vector<LineRendererInput::Line> const& lines) const {
    using T       = typename std::decay<decltype(*lines.begin())>::type;
    auto numBytes = std::min(attributeBuffer_.SizeBytes(), static_cast<int32_t>(std::size(lines) * sizeof(T)));
    attributeBuffer_.Fill(lines.data(), numBytes);
}

} // namespace engine::gl
