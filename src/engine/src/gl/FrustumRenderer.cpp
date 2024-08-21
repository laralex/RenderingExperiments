
#include "engine/gl/FrustumRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

constexpr GLint UNIFORM_MVP_LOCATION   = 0;
constexpr GLint UBO_CONTEXT_BINDING    = 0; // global for GL
constexpr GLint UBO_SHADER_BINDING     = 0; // local for shader
constexpr GLint UNIFORM_COLOR_LOCATION = 100;

constexpr float THICKNESS = 0.03f;
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

struct UboData {
    GLfloat left;
    GLfloat right;
    GLfloat bottom;
    GLfloat top;
    GLfloat near;
    GLfloat far;
    GLfloat thickness;
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

} // namespace

namespace engine::gl {

auto AllocateFrustumRenderer() -> FrustumRenderer {
    constexpr GLint ATTRIB_FRUSTUM_WEIGHTS_LOCATION = 0;
    constexpr GLint ATTRIB_OTHER_WEIGHTS_LOCATION   = 1;

    FrustumRenderer renderer;
    renderer.attributeBuffer = gl::GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertexData, sizeof(vertexData), "FrustumRenderer Vertices");
    renderer.indexBuffer = gl::GpuBuffer::Allocate(
        GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices), "FrustumRenderer Indices");
    renderer.vao = gl::Vao::Allocate("FrustumRenderer");
    (void)gl::VaoMutableCtx{renderer.vao}
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
        .LinkIndices(renderer.indexBuffer, GL_UNSIGNED_BYTE);

    constexpr static int32_t NUM_DEFINES        = 5;
    gl::ShaderDefine const defines[NUM_DEFINES] = {
        {.name = "ATTRIB_FRUSTUM_WEIGHTS", .value = ATTRIB_FRUSTUM_WEIGHTS_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_OTHER_WEIGHTS", .value = ATTRIB_OTHER_WEIGHTS_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_MVP", .value = UNIFORM_MVP_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UBO_FRUSTUM", .value = UBO_SHADER_BINDING, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_COLOR_LOCATION", .value = UNIFORM_COLOR_LOCATION, .type = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/frustum.vert", "data/engine/shaders/constant.frag", CpuView{defines, NUM_DEFINES},
        "FrustumRenderer");
    assert(maybeProgram);
    renderer.program = std::move(*maybeProgram);

    renderer.ubo =
        gl::GpuBuffer::Allocate(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, nullptr, sizeof(UboData), "FrustumRenderer UBO");

    return renderer;
}

void RenderFrustum(
    FrustumRenderer const& renderer, glm::mat4 const& originMvp, Frustum const& frustum, glm::vec4 color) {
    auto programGuard = gl::UniformCtx(renderer.program);
    gl::UniformMatrix4(UNIFORM_MVP_LOCATION, &originMvp[0][0]);
    UboData ubo{
        .left      = frustum.left,
        .right     = frustum.right,
        .bottom    = frustum.bottom,
        .top       = frustum.top,
        .near      = frustum.near,
        .far       = frustum.far,
        .thickness = THICKNESS,
    };
    gl::UniformArray<4>(UNIFORM_COLOR_LOCATION, &color[0], 1);

    renderer.ubo.Fill(&ubo.left, sizeof(ubo));
    GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_CONTEXT_BINDING, renderer.ubo.Id()));
    programGuard.SetUbo(UBO_SHADER_BINDING, UBO_CONTEXT_BINDING);

    auto vaoGuard = VaoCtx{renderer.vao};

    GLCALL(glDisable(GL_CULL_FACE));
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_TRUE));
    GLCALL(glDepthFunc(GL_LEQUAL));

    GLCALL(glDrawElements(GL_TRIANGLES, renderer.vao.IndexCount(), renderer.vao.IndexDataType(), 0));
}

} // namespace engine::gl