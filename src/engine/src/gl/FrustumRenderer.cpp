
#include "engine/gl/FrustumRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

#include "engine_private/Prelude.hpp"

namespace {

constexpr GLint UNIFORM_MVP_LOCATION   = 0;
constexpr GLint UBO_BINDING            = 0; // global for GL
constexpr GLint UNIFORM_COLOR_LOCATION = 100;

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
    alignas(16) glm::vec4 leftRightBottomTop;
    alignas(16) glm::vec4 nearFarThickness;
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

ENGINE_EXPORT auto FrustumRenderer::Allocate(GlContext const& gl) -> FrustumRenderer {
    constexpr GLint ATTRIB_FRUSTUM_WEIGHTS_LOCATION = 0;
    constexpr GLint ATTRIB_OTHER_WEIGHTS_LOCATION   = 1;

    FrustumRenderer renderer;
    renderer.attributeBuffer_ = gl::GpuBuffer::Allocate(
        gl, GL_ARRAY_BUFFER, GL_STATIC_DRAW, CpuMemory<void const>{vertexData, sizeof(vertexData)},
        "FrustumRenderer Vertices");
    renderer.indexBuffer_ = gl::GpuBuffer::Allocate(
        gl, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, CpuMemory<void const>{indices, sizeof(indices)},
        "FrustumRenderer Indices");
    renderer.vao_ = gl::Vao::Allocate(gl, "FrustumRenderer");
    std::ignore = gl::VaoMutableCtx{renderer.vao_}
        .MakeVertexAttribute(
            renderer.attributeBuffer_,
            {.location        = ATTRIB_FRUSTUM_WEIGHTS_LOCATION,
             .valuesPerVertex = 4,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(Vertex),
             .offset          = offsetof(Vertex, isLeftRightBottomTop)})
        .MakeVertexAttribute(
            renderer.attributeBuffer_,
            {.location        = ATTRIB_OTHER_WEIGHTS_LOCATION,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(Vertex),
             .offset          = offsetof(Vertex, isNear)})
        .MakeIndexed(renderer.indexBuffer_, GL_UNSIGNED_BYTE);

    gl::shader::Define const defines[] = {
        {.name = "ATTRIB_FRUSTUM_WEIGHTS", .value = ATTRIB_FRUSTUM_WEIGHTS_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "ATTRIB_OTHER_WEIGHTS", .value = ATTRIB_OTHER_WEIGHTS_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "UNIFORM_MVP", .value = UNIFORM_MVP_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "UBO_FRUSTUM", .value = UBO_BINDING, .type = gl::shader::Define::INT32},
        {.name = "UNIFORM_COLOR_LOCATION", .value = UNIFORM_COLOR_LOCATION, .type = gl::shader::Define::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        gl, "data/engine/shaders/frustum.vert", "data/engine/shaders/constant.frag",
        CpuView{defines, std::size(defines)}, "FrustumRenderer");
    assert(maybeProgram);
    renderer.program_     = std::move(*maybeProgram);
    renderer.uboLocation_ = UniformCtx::GetUboLocation(renderer.program_, "Ubo");

    renderer.ubo_ = gl::GpuBuffer::Allocate(
        gl, GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, CpuMemory<void const>{nullptr, sizeof(UboData)}, "FrustumRenderer UBO");

    return renderer;
}

ENGINE_EXPORT void FrustumRenderer::Render(
    glm::mat4 const& originMvp, Frustum const& frustum, glm::vec4 color, float thickness) const {
    auto programGuard = gl::UniformCtx(program_);
    UboData data{
        .leftRightBottomTop = {frustum.left, frustum.right, frustum.bottom, frustum.top},
        .nearFarThickness   = {frustum.near, frustum.far, thickness*2.0f, 0.0},
    };
    ubo_.Fill(CpuMemory<GLvoid const>{&data, sizeof(data)});
    GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BINDING, ubo_.Id()));
    // programGuard.SetUbo(uboLocation_, UBO_BINDING);

    programGuard.SetUniformValue4(UNIFORM_COLOR_LOCATION, glm::value_ptr(color));
    programGuard.SetUniformMatrix4x4(UNIFORM_MVP_LOCATION, glm::value_ptr(originMvp));

    GLCALL(glDisable(GL_CULL_FACE));
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_TRUE));
    GLCALL(glDepthFunc(GL_LEQUAL));

    RenderVao(vao_);
}

} // namespace engine::gl