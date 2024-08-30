
#include "engine/gl/BoxRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

constexpr float THICKNESS = 0.03f;
constexpr float OUT_BEGIN = -0.5f;
constexpr float OUT_END   = 0.5f;
constexpr float IN_BEGIN  = -0.5f;
constexpr float IN_END    = 0.5f;

struct Vertex {
    float position[3];
    float innerMarker;
};

// clang-format off
constexpr Vertex vertexData[] = {
    // outer cover
    {{ OUT_BEGIN, OUT_BEGIN, OUT_BEGIN}, 0.0f}, // 0
    {{ OUT_END, OUT_BEGIN, OUT_BEGIN}, 0.0f}, // 1
    {{ OUT_BEGIN, OUT_END, OUT_BEGIN, }, 0.0f}, // 2
    {{ OUT_END, OUT_END, OUT_BEGIN, }, 0.0f}, // 3
    {{ OUT_BEGIN, OUT_BEGIN, OUT_END, }, 0.0f}, // 4
    {{ OUT_END, OUT_BEGIN, OUT_END, }, 0.0f}, // 5
    {{ OUT_BEGIN, OUT_END, OUT_END, }, 0.0f}, // 6
    {{ OUT_END, OUT_END, OUT_END, }, 0.0f}, // 7
    // inner cover
    {{ IN_BEGIN, IN_BEGIN, IN_BEGIN, }, 1.0f}, // 8
    {{ IN_END, IN_BEGIN, IN_BEGIN, }, 1.0f}, // 9
    {{ IN_BEGIN, IN_END, IN_BEGIN, }, 1.0f}, // 10
    {{ IN_END, IN_END, IN_BEGIN, }, 1.0f}, // 11
    {{ IN_BEGIN, IN_BEGIN, IN_END, }, 1.0f}, // 12
    {{ IN_END, IN_BEGIN, IN_END, }, 1.0f}, // 13
    {{ IN_BEGIN, IN_END, IN_END, }, 1.0f}, // 14
    {{ IN_END, IN_END, IN_END, }, 1.0f}, // 15
};

constexpr uint8_t X0Y0Z0_OUT = 0;
constexpr uint8_t X1Y0Z0_OUT = 1;
constexpr uint8_t X0Y1Z0_OUT = 2;
constexpr uint8_t X1Y1Z0_OUT = 3;
constexpr uint8_t X0Y0Z1_OUT = 4;
constexpr uint8_t X1Y0Z1_OUT = 5;
constexpr uint8_t X0Y1Z1_OUT = 6;
constexpr uint8_t X1Y1Z1_OUT = 7;
constexpr uint8_t X0Y0Z0_IN = 8;
constexpr uint8_t X1Y0Z0_IN = 9;
constexpr uint8_t X0Y1Z0_IN = 10;
constexpr uint8_t X1Y1Z0_IN = 11;
constexpr uint8_t X0Y0Z1_IN = 12;
constexpr uint8_t X1Y0Z1_IN = 13;
constexpr uint8_t X0Y1Z1_IN = 14;
constexpr uint8_t X1Y1Z1_IN = 15;

constexpr uint8_t indices[] = {
    // X0Y0Z0 - X1Y0Z0
    X0Y0Z0_OUT, X1Y0Z0_OUT, X0Y0Z0_IN,
    X0Y0Z0_IN,  X1Y0Z0_IN,  X1Y0Z0_OUT,
    // X0Y0Z0 - X0Y1Z0
    X0Y0Z0_OUT, X0Y1Z0_OUT, X0Y0Z0_IN,
    X0Y0Z0_IN,  X0Y1Z0_IN,  X0Y1Z0_OUT,
    // X1Y0Z0 - X1Y1Z0
    X1Y0Z0_OUT, X1Y1Z0_OUT, X1Y0Z0_IN,
    X1Y0Z0_IN,  X1Y1Z0_IN,  X1Y1Z0_OUT,
    // X0Y1Z0 - X1Y1Z0
    X0Y1Z0_OUT, X1Y1Z0_OUT, X0Y1Z0_IN,
    X0Y1Z0_IN,  X1Y1Z0_IN,  X1Y1Z0_OUT,

    // X0Y0Z0 - X0Y0Z1
    X0Y0Z0_OUT, X0Y0Z1_OUT, X0Y0Z0_IN,
    X0Y0Z0_IN,  X0Y0Z1_IN,  X0Y0Z1_OUT,
    // X1Y0Z0 - X1Y0Z1
    X1Y0Z0_OUT, X1Y0Z1_OUT, X1Y0Z0_IN,
    X1Y0Z0_IN,  X1Y0Z1_IN,  X1Y0Z1_OUT,
    // X0Y1Z0 - X0Y1Z1
    X0Y1Z0_OUT, X0Y1Z1_OUT, X0Y1Z0_IN,
    X0Y1Z0_IN,  X0Y1Z1_IN,  X0Y1Z1_OUT,
    // X1Y1Z0 - X1Y1Z1
    X1Y1Z0_OUT, X1Y1Z1_OUT, X1Y1Z0_IN,
    X1Y1Z0_IN,  X1Y1Z1_IN,  X1Y1Z1_OUT,

    // X0Y0Z1 - X1Y0Z1
    X0Y0Z1_OUT, X1Y0Z1_OUT, X0Y0Z1_IN,
    X0Y0Z1_IN,  X1Y0Z1_IN,  X1Y0Z1_OUT,
    // X0Y0Z1 - X0Y1Z1
    X0Y0Z1_OUT, X0Y1Z1_OUT, X0Y0Z1_IN,
    X0Y0Z1_IN,  X0Y1Z1_IN,  X0Y1Z1_OUT,
    // X1Y0Z1 - X1Y1Z1
    X1Y0Z1_OUT, X1Y1Z1_OUT, X1Y0Z1_IN,
    X1Y0Z1_IN,  X1Y1Z1_IN,  X1Y1Z1_OUT,
    // X0Y1Z1 - X1Y1Z1
    X0Y1Z1_OUT, X1Y1Z1_OUT, X0Y1Z1_IN,
    X0Y1Z1_IN,  X1Y1Z1_IN,  X1Y1Z1_OUT,
};
// clang-format on

constexpr GLint UNIFORM_MVP_LOCATION       = 0;
constexpr GLint UNIFORM_THICKNESS_LOCATION = 1;
constexpr GLint UNIFORM_COLOR_LOCATION     = 2;

} // namespace

namespace engine::gl {

auto BoxRenderer::Allocate() -> BoxRenderer {
    constexpr GLint ATTRIB_POSITION_LOCATION     = 0;
    constexpr GLint ATTRIB_INNER_MARKER_LOCATION = 1;

    BoxRenderer renderer;
    renderer.attributeBuffer_ = gl::GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertexData, sizeof(vertexData), "BoxRenderer Vertices");
    renderer.indexBuffer_ = gl::GpuBuffer::Allocate(
        GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices), "BoxRenderer Indices");
    renderer.vao_ = gl::Vao::Allocate("BoxRenderer");
    (void)gl::VaoMutableCtx{renderer.vao_}
        .MakeVertexAttribute(
            renderer.attributeBuffer_,
            {.location        = ATTRIB_POSITION_LOCATION,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(Vertex),
             .offset          = offsetof(Vertex, position)})
        .MakeVertexAttribute(
            renderer.attributeBuffer_,
            {.location        = ATTRIB_INNER_MARKER_LOCATION,
             .valuesPerVertex = 1,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(Vertex),
             .offset          = offsetof(Vertex, innerMarker)})
        .MakeIndexed(renderer.indexBuffer_, GL_UNSIGNED_BYTE);

    gl::ShaderDefine const defines[] = {
        {.name = "ATTRIB_POSITION_LOCATION", .value = ATTRIB_POSITION_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name  = "ATTRIB_INNER_MARKER_LOCATION",
         .value = ATTRIB_INNER_MARKER_LOCATION,
         .type  = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_MVP_LOCATION", .value = UNIFORM_MVP_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_THICKNESS_LOCATION", .value = UNIFORM_THICKNESS_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_COLOR_LOCATION", .value = UNIFORM_COLOR_LOCATION, .type = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/box.vert", "data/engine/shaders/constant.frag", CpuView{defines, std::size(defines)},
        "BoxRenderer");
    assert(maybeProgram);
    renderer.program_ = std::move(*maybeProgram);

    auto programGuard = UniformCtx{renderer.program_};
    programGuard.SetUniformValue1(UNIFORM_THICKNESS_LOCATION, THICKNESS);

    return renderer;
}

void BoxRenderer::Render(glm::mat4 const& centerMvp, glm::vec4 color) const {
    auto programGuard = gl::UniformCtx(program_);
    programGuard.SetUniformMatrix4x4(UNIFORM_MVP_LOCATION, glm::value_ptr(centerMvp));
    programGuard.SetUniformValue4(UNIFORM_COLOR_LOCATION, glm::value_ptr(color));

    GLCALL(glDisable(GL_CULL_FACE));
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_TRUE));
    GLCALL(glDepthFunc(GL_LEQUAL));

    RenderVao(vao_);
}

} // namespace engine::gl