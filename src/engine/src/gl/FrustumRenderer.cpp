
#include "engine/gl/FrustumRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

constexpr float THICKNESS = 0.1f;
constexpr float OUT_BEGIN = 0.5f;
constexpr float OUT_END   = 1.0f;
constexpr float IN_BEGIN  = 0.5f;
constexpr float IN_END    = 1.0f;

struct Vertex {
    float isLeftRightBottomTop[4];
    float isNear;
    float isFar;
    float isInner;
};

// clang-format off
constexpr Vertex vertexData[] = {
    // outer cover
    {{1.0f, 0.0f, 1.0f, 0.0f}, 1.0f, 0.0f, 0.0f}, // 0
    {{0.0f, 1.0f, 1.0f, 0.0f}, 1.0f, 0.0f, 0.0f}, // 1
    {{1.0f, 0.0f, 0.0f, 1.0f}, 1.0f, 0.0f, 0.0f}, // 2
    {{0.0f, 1.0f, 0.0f, 1.0f}, 1.0f, 0.0f, 0.0f}, // 3
    {{1.0f, 0.0f, 1.0f, 0.0f}, 0.0f, 1.0f, 0.0f}, // 4
    {{0.0f, 1.0f, 1.0f, 0.0f}, 0.0f, 1.0f, 0.0f}, // 5
    {{1.0f, 0.0f, 0.0f, 1.0f}, 0.0f, 1.0f, 0.0f}, // 6
    {{0.0f, 1.0f, 0.0f, 1.0f}, 0.0f, 1.0f, 0.0f}, // 7
    // inner cover
    {{1.0f, 0.0f, 1.0f, 0.0f}, 1.0f, 0.0f, 1.0f}, // 8
    {{0.0f, 1.0f, 1.0f, 0.0f}, 1.0f, 0.0f, 1.0f}, // 9
    {{1.0f, 0.0f, 0.0f, 1.0f}, 1.0f, 0.0f, 1.0f}, // 10
    {{0.0f, 1.0f, 0.0f, 1.0f}, 1.0f, 0.0f, 1.0f}, // 11
    {{1.0f, 0.0f, 1.0f, 0.0f}, 0.0f, 1.0f, 1.0f}, // 12
    {{0.0f, 1.0f, 1.0f, 0.0f}, 0.0f, 1.0f, 1.0f}, // 13
    {{1.0f, 0.0f, 0.0f, 1.0f}, 0.0f, 1.0f, 1.0f}, // 14
    {{0.0f, 1.0f, 0.0f, 1.0f}, 0.0f, 1.0f, 1.0f}, // 15
    // origin
    {{ 0.0f, 0.0f, 0.0f, 0.0f}, 0.0f, 0.0f, 0.0f}, // 16
    {{ 1.0f, 0.0f, 1.0f, 0.0f}, 0.0f, 0.0f, 1.0f}, // 17 for X0Y0
    {{ 0.0f, 1.0f, 1.0f, 0.0f}, 0.0f, 0.0f, 1.0f}, // 18 for X1Y0
    {{ 1.0f, 0.0f, 0.0f, 1.0f}, 0.0f, 0.0f, 1.0f}, // 19 for X0Y1
    {{ 0.0f, 1.0f, 0.0f, 1.0f}, 0.0f, 0.0f, 1.0f}, // 20 for X1Y1
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
constexpr uint8_t ORIGIN_OUT = 16;
constexpr uint8_t ORIGIN_X0Y0_IN = 17;
constexpr uint8_t ORIGIN_X1Y0_IN = 18;
constexpr uint8_t ORIGIN_X0Y1_IN = 19;
constexpr uint8_t ORIGIN_X1Y1_IN = 20;

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

    // X0Y0Z0 - ORIGIN
    X0Y0Z0_OUT, ORIGIN_OUT,     X0Y0Z0_IN,
    X0Y0Z0_IN,  ORIGIN_X0Y0_IN, ORIGIN_OUT,
    // X1Y0Z0 - ORIGIN
    X1Y0Z0_OUT, ORIGIN_OUT,     X1Y0Z0_IN,
    X1Y0Z0_IN,  ORIGIN_X1Y0_IN, ORIGIN_OUT,
    // X0Y1Z0 - ORIGIN
    X0Y1Z0_OUT, ORIGIN_OUT,     X0Y1Z0_IN,
    X0Y1Z0_IN,  ORIGIN_X0Y1_IN, ORIGIN_OUT,
    // X1Y1Z0 - ORIGIN
    X1Y1Z0_OUT, ORIGIN_OUT,     X1Y1Z0_IN,
    X1Y1Z0_IN,  ORIGIN_X1Y1_IN, ORIGIN_OUT,
};
// clang-format on

constexpr int32_t UNIFORM_MVP_LOCATION       = 0;
constexpr int32_t UNIFORM_THICKNESS_LOCATION = 1;
constexpr int32_t UNIFORM_FRUSTUM_LOCATION   = 2;
constexpr int32_t UNIFORM_NEAR_FAR_LOCATION  = 3;
constexpr int32_t UNIFORM_COLOR_LOCATION     = 4;

} // namespace

namespace engine::gl {

auto AllocateFrustumRenderer() -> FrustumRenderer {
    constexpr int32_t ATTRIB_FRUSTUM_WEIGHTS_LOCATION = 0;
    constexpr int32_t ATTRIB_OTHER_WEIGHTS_LOCATION   = 1;

    FrustumRenderer renderer;
    renderer.attributeBuffer = gl::GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertexData, sizeof(vertexData), "FrustumRenderer Vertices");
    renderer.indexBuffer = gl::GpuBuffer::Allocate(
        GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices), "FrustumRenderer Indices");
    renderer.vao = gl::Vao::Allocate("FrustumRenderer");
    (void)gl::VaoCtx{renderer.vao}
        .LinkVertexAttribute(
            renderer.attributeBuffer,
            {.index           = ATTRIB_FRUSTUM_WEIGHTS_LOCATION,
             .valuesPerVertex = 4,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(Vertex),
             .offset          = offsetof(Vertex, isLeftRightBottomTop)})
        .LinkVertexAttribute(
            renderer.attributeBuffer,
            {.index           = ATTRIB_OTHER_WEIGHTS_LOCATION,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(Vertex),
             .offset          = offsetof(Vertex, isNear)})
        .LinkIndices(renderer.indexBuffer);

    constexpr static int32_t NUM_VDEFINES   = 6;
    gl::ShaderDefine vdefines[NUM_VDEFINES] = {
        {.name = "ATTRIB_FRUSTUM_WEIGHTS", .value = ATTRIB_FRUSTUM_WEIGHTS_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_OTHER_WEIGHTS", .value = ATTRIB_OTHER_WEIGHTS_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_MVP", .value = UNIFORM_MVP_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_FRUSTUM", .value = UNIFORM_FRUSTUM_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_NEAR_FAR", .value = UNIFORM_NEAR_FAR_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_THICKNESS", .value = UNIFORM_THICKNESS_LOCATION, .type = gl::ShaderDefine::INT32},
    };

    constexpr static int32_t NUM_FDEFINES   = 1;
    gl::ShaderDefine fdefines[NUM_FDEFINES] = {
        {.name = "UNIFORM_COLOR_LOCATION", .value = UNIFORM_COLOR_LOCATION, .type = gl::ShaderDefine::INT32},
    };
    std::string vertexShaderCode   = gl::LoadShaderCode("data/engine/shaders/frustum.vert", vdefines, NUM_VDEFINES);
    std::string fragmentShaderCode = gl::LoadShaderCode("data/engine/shaders/constant.frag", fdefines, NUM_FDEFINES);
    GLuint vertexShader            = gl::CompileShader(GL_VERTEX_SHADER, vertexShaderCode);
    GLuint fragmentShader          = gl::CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
    renderer.program               = *gl::GpuProgram::Allocate(vertexShader, fragmentShader, "FrustumRenderer");
    auto programGuard              = UniformCtx{renderer.program};
    gl::UniformValue1(UNIFORM_THICKNESS_LOCATION, THICKNESS);

    GLCALL(glDeleteShader(vertexShader));
    GLCALL(glDeleteShader(fragmentShader));
    return renderer;
}

void RenderFrustum(FrustumRenderer const& renderer, glm::mat4 const& originMvp, glm::vec4 color) {
    auto programGuard = gl::UniformCtx(renderer.program);
    gl::UniformMatrix4(UNIFORM_MVP_LOCATION, &originMvp[0][0]);
    gl::UniformArray<4>(UNIFORM_COLOR_LOCATION, &color[0], 1);
    glm::vec4 frustum{0.5f, 0.2f, 0.5f, 0.2f};
    gl::UniformArray<4>(UNIFORM_FRUSTUM_LOCATION, &frustum[0], 1);
    glm::vec2 nearFar{0.5f, 2.0f};
    gl::UniformArray<2>(UNIFORM_NEAR_FAR_LOCATION, &nearFar[0], 1);

    auto vaoGuard = VaoCtx{renderer.vao};

    GLCALL(glDisable(GL_CULL_FACE));
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_TRUE));
    GLCALL(glDepthFunc(GL_LEQUAL));

    GLCALL(glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_BYTE, 0));
}

} // namespace engine::gl