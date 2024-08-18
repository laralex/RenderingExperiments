
#include "engine/gl/BoxRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

constexpr float THICKNESS    = 0.03f;
constexpr float OUT_BEGIN    = -0.5f;
constexpr float OUT_END      =  0.5f;
constexpr float IN_BEGIN     = -0.5f;
constexpr float IN_END       =  0.5f;
constexpr float BIAS         = 0.00f;

constexpr float ZERO[3]      = { 0.0f, 0.0f, 0.0f };
constexpr float AXIS_X[3]    = { 1.0f, 0.0f, 0.0f };
constexpr float AXIS_Y[3]    = { 0.0f, 1.0f, 0.0f };
constexpr float AXIS_Z[3]    = { 0.0f, 0.0f, 1.0f };

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

constexpr int32_t UNIFORM_MVP_LOCATION = 0;
constexpr int32_t UNIFORM_THICKNESS_LOCATION = 1;
constexpr int32_t UNIFORM_COLOR_LOCATION = 2;

} // namespace anonymous

namespace engine::gl {

auto AllocateBoxRenderer() -> BoxRenderer {
    constexpr int32_t ATTRIB_POSITION_LOCATION = 0;
    constexpr int32_t ATTRIB_INNER_MARKER_LOCATION = 1;

    BoxRenderer renderer;
    renderer.attributeBuffer = gl::GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertexData, sizeof(vertexData), "BoxRenderer Vertices");
    renderer.indexBuffer = gl::GpuBuffer::Allocate(
        GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices), "BoxRenderer Indices");
    renderer.vao = gl::Vao::Allocate("BoxRenderer");
    (void)gl::VaoCtx{renderer.vao}
        .LinkVertexAttribute(
            renderer.attributeBuffer,
            {.index           = ATTRIB_POSITION_LOCATION,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(Vertex),
             .offset          = offsetof(Vertex, position)})
        .LinkVertexAttribute(
            renderer.attributeBuffer,
            {.index           = ATTRIB_INNER_MARKER_LOCATION,
             .valuesPerVertex = 1,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(Vertex),
             .offset          = offsetof(Vertex, innerMarker)})
        .LinkIndices(renderer.indexBuffer);

    constexpr static int32_t NUM_VDEFINES   = 4;
    gl::ShaderDefine vdefines[NUM_VDEFINES] = {
        {.name = "ATTRIB_POSITION_LOCATION", .value = ATTRIB_POSITION_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_INNER_MARKER_LOCATION", .value = ATTRIB_INNER_MARKER_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_MVP_LOCATION", .value = UNIFORM_MVP_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_THICKNESS_LOCATION", .value = UNIFORM_THICKNESS_LOCATION, .type = gl::ShaderDefine::INT32},
    };
    constexpr static int32_t NUM_FDEFINES   = 1;
    gl::ShaderDefine fdefines[NUM_FDEFINES] = {
        {.name = "UNIFORM_COLOR_LOCATION", .value = UNIFORM_COLOR_LOCATION, .type = gl::ShaderDefine::INT32},
    };
    std::string vertexShaderCode   = gl::LoadShaderCode("data/engine/shaders/box.vert", vdefines, NUM_VDEFINES);
    std::string fragmentShaderCode = gl::LoadShaderCode("data/engine/shaders/constant.frag", fdefines, NUM_FDEFINES);
    GLuint vertexShader            = gl::CompileShader(GL_VERTEX_SHADER, vertexShaderCode);
    GLuint fragmentShader          = gl::CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
    renderer.program           = *gl::GpuProgram::Allocate(vertexShader, fragmentShader, "BoxRenderer");
    auto programGuard = UniformCtx{renderer.program};
    gl::UniformValue1(UNIFORM_THICKNESS_LOCATION, THICKNESS);

    GLCALL(glDeleteShader(vertexShader));
    GLCALL(glDeleteShader(fragmentShader));
    return renderer;
}

void RenderBox(BoxRenderer const& renderer, glm::mat4 const& centerMvp, glm::vec4 color) {
    auto programGuard = gl::UniformCtx(renderer.program);
    gl::UniformMatrix4(UNIFORM_MVP_LOCATION, &centerMvp[0][0]);
    gl::UniformArray<4>(UNIFORM_COLOR_LOCATION, &color[0], 1);

    auto vaoGuard = VaoCtx{renderer.vao};

    GLCALL(glDisable(GL_CULL_FACE));
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_TRUE));
    GLCALL(glDepthFunc(GL_LEQUAL));

    GLCALL(glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_BYTE, 0));
}

} // namespace engine::gl