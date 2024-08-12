#include "engine/AxesRenderer.hpp"

#include "engine/Assets.hpp"
#include "engine/gl/Vao.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

struct AxesRenderer {
    engine::gl::Vao vao;
    engine::gl::GpuBuffer attributeBuffer;
    engine::gl::GpuBuffer indexBuffer;
    engine::gl::GpuProgram program;
};

AxesRenderer g_axesRenderer;

constexpr float LENGTH = 1.0f;
constexpr float ARROW_LENGTH = 0.1f;
constexpr float ARROW_WIDTH = 0.03;
constexpr float LINE_END = LENGTH - ARROW_LENGTH;
constexpr float THICKNESS = LENGTH * 0.02f;

constexpr float vertexData[] = {
    // X-axis body
    0.0f , -THICKNESS, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f , THICKNESS, 0.0f, 1.0f, 0.0f, 0.0f,
    LINE_END , -THICKNESS, 0.0f, 1.0f, 0.0f, 0.0f,
    LINE_END , THICKNESS, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f , 0.0f, -THICKNESS, 1.0f, 0.0f, 0.0f,
    0.0f , 0.0f, THICKNESS, 1.0f, 0.0f, 0.0f,
    LINE_END , 0.0f, -THICKNESS, 1.0f, 0.0f, 0.0f,
    LINE_END , 0.0f, THICKNESS, 1.0f, 0.0f, 0.0f,
    // Y-axis body
    -THICKNESS, 0.0f , 0.0f, 0.0f, 0.0f, 1.0f,
    THICKNESS, 0.0f , 0.0f, 0.0f, 0.0f, 1.0f,
    -THICKNESS, LINE_END , 0.0f, 0.0f, 0.0f, 1.0f,
    THICKNESS, LINE_END , 0.0f, 0.0f, 0.0f, 1.0f,
    0.0f , 0.0f, -THICKNESS, 0.0f, 0.0f, 1.0f,
    0.0f , 0.0f, THICKNESS, 0.0f, 0.0f, 1.0f,
    0.0f , LINE_END, -THICKNESS, 0.0f, 0.0f, 1.0f,
    0.0f , LINE_END, THICKNESS, 0.0f, 0.0f, 1.0f,
    // Z-axis body
    -THICKNESS, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f,
    THICKNESS, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f,
    -THICKNESS, 0.0f , LINE_END, 0.0f, 1.0f, 0.0f,
    THICKNESS, 0.0f , LINE_END, 0.0f, 1.0f, 0.0f,
    0.0f , -THICKNESS, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f , THICKNESS, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f , -THICKNESS, LINE_END, 0.0f, 1.0f, 0.0f,
    0.0f , THICKNESS, LINE_END, 0.0f, 1.0f, 0.0f,
    // X-axis arrow
    LINE_END, -ARROW_WIDTH, -ARROW_WIDTH, 1.0f, 0.0f, 0.0f, // 0
    LINE_END, ARROW_WIDTH,  -ARROW_WIDTH, 1.0f, 0.0f, 0.0f, // 1
    LINE_END, -ARROW_WIDTH, ARROW_WIDTH, 1.0f, 0.0f, 0.0f, // 2
    LINE_END, ARROW_WIDTH,  ARROW_WIDTH, 1.0f, 0.0f, 0.0f, // 3
    LENGTH, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 4
    // Y-axis arrow
    -ARROW_WIDTH, LINE_END, -ARROW_WIDTH, 0.0f, 0.0f, 1.0f,
    ARROW_WIDTH,  LINE_END, -ARROW_WIDTH, 0.0f, 0.0f, 1.0f,
    -ARROW_WIDTH, LINE_END, ARROW_WIDTH, 0.0f, 0.0f, 1.0f,
    ARROW_WIDTH,  LINE_END, ARROW_WIDTH, 0.0f, 0.0f, 1.0f,
    0.0f, LENGTH, 0.0f, 0.0f, 0.0f, 1.0f,
    // Z-axis arrow
    -ARROW_WIDTH, -ARROW_WIDTH, LINE_END,  0.0f, 1.0f, 0.0f,
    ARROW_WIDTH,  -ARROW_WIDTH, LINE_END, 0.0f, 1.0f,  0.0f,
    -ARROW_WIDTH, ARROW_WIDTH, LINE_END,0.0f, 1.0f, 0.0f,
    ARROW_WIDTH,  ARROW_WIDTH, LINE_END,0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, LENGTH, 0.0f, 1.0f, 0.0f,
};

constexpr uint8_t XY_IDX_OFFSET = 0;
constexpr uint8_t XZ_IDX_OFFSET = 4;
constexpr uint8_t YX_IDX_OFFSET = 8;
constexpr uint8_t YZ_IDX_OFFSET = 12;
constexpr uint8_t ZX_IDX_OFFSET = 16;
constexpr uint8_t ZY_IDX_OFFSET = 20;
constexpr uint8_t X_ARROW_IDX_OFFSET = 24;
constexpr uint8_t Y_ARROW_IDX_OFFSET = 29;
constexpr uint8_t Z_ARROW_IDX_OFFSET = 34;

constexpr uint8_t indices[] = {
    XY_IDX_OFFSET, XY_IDX_OFFSET+2, XY_IDX_OFFSET+1,
    XY_IDX_OFFSET+3, XY_IDX_OFFSET+1, XY_IDX_OFFSET+2,
    XZ_IDX_OFFSET, XZ_IDX_OFFSET+2, XZ_IDX_OFFSET+1,
    XZ_IDX_OFFSET+3, XZ_IDX_OFFSET+1, XZ_IDX_OFFSET+2,
    YX_IDX_OFFSET, YX_IDX_OFFSET+2, YX_IDX_OFFSET+1,
    YX_IDX_OFFSET+3, YX_IDX_OFFSET+1, YX_IDX_OFFSET+2,
    YZ_IDX_OFFSET, YZ_IDX_OFFSET+2, YZ_IDX_OFFSET+1,
    YZ_IDX_OFFSET+3, YZ_IDX_OFFSET+1, YZ_IDX_OFFSET+2,
    ZX_IDX_OFFSET, ZX_IDX_OFFSET+2, ZX_IDX_OFFSET+1,
    ZX_IDX_OFFSET+3, ZX_IDX_OFFSET+1, ZX_IDX_OFFSET+2,
    ZY_IDX_OFFSET, ZY_IDX_OFFSET+2, ZY_IDX_OFFSET+1,
    ZY_IDX_OFFSET+3, ZY_IDX_OFFSET+1, ZY_IDX_OFFSET+2,
    X_ARROW_IDX_OFFSET, X_ARROW_IDX_OFFSET+1, X_ARROW_IDX_OFFSET+4,
    X_ARROW_IDX_OFFSET, X_ARROW_IDX_OFFSET+2, X_ARROW_IDX_OFFSET+4,
    X_ARROW_IDX_OFFSET+1, X_ARROW_IDX_OFFSET+3, X_ARROW_IDX_OFFSET+4,
    X_ARROW_IDX_OFFSET+2, X_ARROW_IDX_OFFSET+3, X_ARROW_IDX_OFFSET+4,
    Y_ARROW_IDX_OFFSET, Y_ARROW_IDX_OFFSET+1, Y_ARROW_IDX_OFFSET+4,
    Y_ARROW_IDX_OFFSET, Y_ARROW_IDX_OFFSET+2, Y_ARROW_IDX_OFFSET+4,
    Y_ARROW_IDX_OFFSET+1, Y_ARROW_IDX_OFFSET+3, Y_ARROW_IDX_OFFSET+4,
    Y_ARROW_IDX_OFFSET+2, Y_ARROW_IDX_OFFSET+3, Y_ARROW_IDX_OFFSET+4,
    Z_ARROW_IDX_OFFSET, Z_ARROW_IDX_OFFSET+1, Z_ARROW_IDX_OFFSET+4,
    Z_ARROW_IDX_OFFSET, Z_ARROW_IDX_OFFSET+2, Z_ARROW_IDX_OFFSET+4,
    Z_ARROW_IDX_OFFSET+1, Z_ARROW_IDX_OFFSET+3, Z_ARROW_IDX_OFFSET+4,
    Z_ARROW_IDX_OFFSET+2, Z_ARROW_IDX_OFFSET+3, Z_ARROW_IDX_OFFSET+4,
};

constexpr int32_t UNIFORM_MVP_LOCATION   = 0;

} // namespace anonymous

namespace engine::gl {

void AllocateAxesRenderer() {
    constexpr int32_t ATTRIB_POSITION_LOCATION = 0;
    constexpr int32_t ATTRIB_COLOR_LOCATION = 1;
    g_axesRenderer.attributeBuffer =
            gl::GpuBuffer::Allocate(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertexData, sizeof(vertexData), "AxesRenderer Vertices");
    g_axesRenderer.indexBuffer =
        gl::GpuBuffer::Allocate(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices), "AxesRenderer Indices");
    g_axesRenderer.vao = gl::Vao::Allocate("AxesRenderer");
    g_axesRenderer.vao.LinkVertexAttribute(
        g_axesRenderer.attributeBuffer,
        {.index           = ATTRIB_POSITION_LOCATION,
            .valuesPerVertex = 3,
            .datatype        = GL_FLOAT,
            .normalized      = GL_FALSE,
            .stride          = 6 * sizeof(float),
            .offset          = 0});
    g_axesRenderer.vao.LinkVertexAttribute(
        g_axesRenderer.attributeBuffer,
        {.index           = ATTRIB_COLOR_LOCATION,
            .valuesPerVertex = 3,
            .datatype        = GL_FLOAT,
            .normalized      = GL_FALSE,
            .stride          = 6 * sizeof(float),
            .offset          = 3 * sizeof(float)});
    g_axesRenderer.vao.LinkIndices(g_axesRenderer.indexBuffer);

    constexpr static int32_t NUM_VDEFINES   = 3;
    gl::ShaderDefine vdefines[NUM_VDEFINES] = {
        {.name = "ATTRIB_POSITION_LOCATION", .value = ATTRIB_POSITION_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_COLOR_LOCATION", .value = ATTRIB_COLOR_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_MVP_LOCATION", .value = UNIFORM_MVP_LOCATION, .type = gl::ShaderDefine::INT32},
    };
    std::string vertexShaderCode = gl::LoadShaderCode("data/engine/shaders/axes.vert", vdefines, NUM_VDEFINES);
    std::string fragmentShaderCode = gl::LoadShaderCode("data/engine/shaders/axes.frag", nullptr, 0);
    GLuint vertexShader            = gl::CompileShader(GL_VERTEX_SHADER, vertexShaderCode);
    GLuint fragmentShader          = gl::CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
    g_axesRenderer.program                   = *gl::GpuProgram::Allocate(vertexShader, fragmentShader, "AxesRenderer");
    GLCALL(glDeleteShader(vertexShader));
    GLCALL(glDeleteShader(fragmentShader));
}

void RenderAxes(glm::mat4 const& mvp) {
    auto programGuard = gl::UniformCtx(g_axesRenderer.program);
    gl::UniformMatrix4(UNIFORM_MVP_LOCATION, &mvp[0][0]);
    GLCALL(glDisable(GL_CULL_FACE));
    GLCALL(glBindVertexArray(g_axesRenderer.vao.Id()));
    GLCALL(glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_BYTE, 0));
}

} // namespace engine::gl