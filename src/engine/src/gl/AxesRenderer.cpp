
#include "engine/gl/AxesRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Context.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

#include "engine_private/Prelude.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace {

constexpr float LENGTH       = 1.0f;
constexpr float ARROW_LENGTH = 0.1f;
constexpr float ARROW_WIDTH  = 0.03;
constexpr float BIAS         = 0.00f;
constexpr float LINE_END     = LENGTH - ARROW_LENGTH;
constexpr float THICKNESS    = LENGTH * 0.02f;

struct Vertex {
    float position[3];
    int32_t colorIdx;
};

// clang-format off
constexpr Vertex vertexData[] = {
    // X-axis body
    {{ BIAS+THICKNESS, -THICKNESS, BIAS, }, 0},
    {{ -BIAS+THICKNESS, THICKNESS, BIAS, }, 0},
    {{ LINE_END, -THICKNESS, 0.0f, }, 0},
    {{ LINE_END, THICKNESS, 0.0f, }, 0},
    {{ BIAS+THICKNESS, -BIAS, -THICKNESS, }, 0},
    {{ -BIAS+THICKNESS, -BIAS, THICKNESS, }, 0},
    {{ LINE_END, 0.0f, -THICKNESS, }, 0},
    {{ LINE_END, 0.0f, THICKNESS, }, 0},
    // Y-axis body
    {{ -THICKNESS, BIAS+THICKNESS, BIAS, }, 1},
    {{ THICKNESS, -BIAS+THICKNESS, BIAS, }, 1},
    {{ -THICKNESS, LINE_END, 0.0f, }, 1},
    {{ THICKNESS, LINE_END, 0.0f, }, 1},
    {{ -BIAS, BIAS+THICKNESS, -THICKNESS, }, 1},
    {{ -BIAS, -BIAS+THICKNESS, THICKNESS, }, 1},
    {{ 0.0f, LINE_END, -THICKNESS, }, 1},
    {{ 0.0f, LINE_END, THICKNESS, }, 1},
    // Z-axis body
    {{ -THICKNESS, BIAS, BIAS+THICKNESS, }, 2},
    {{ THICKNESS, BIAS, -BIAS+THICKNESS, }, 2},
    {{ -THICKNESS, 0.0f, LINE_END, }, 2},
    {{ THICKNESS, 0.0f, LINE_END, }, 2},
    {{ -BIAS, -THICKNESS, BIAS+THICKNESS, }, 2},
    {{ -BIAS, THICKNESS, -BIAS+THICKNESS, }, 2},
    {{ 0.0f, -THICKNESS, LINE_END, }, 2},
    {{ 0.0f, THICKNESS, LINE_END, }, 2},
    // X-axis arrow
    {{ LINE_END, -ARROW_WIDTH, -ARROW_WIDTH, }, 0}, // 0
    {{ LINE_END, ARROW_WIDTH, -ARROW_WIDTH, }, 0}, // 1
    {{ LINE_END, -ARROW_WIDTH, ARROW_WIDTH, }, 0}, // 2
    {{ LINE_END, ARROW_WIDTH, ARROW_WIDTH, }, 0}, // 3
    {{ LENGTH, 0.0f, 0.0f, }, 0}, // 4
    // Y-axis arrow
    {{ -ARROW_WIDTH, LINE_END, -ARROW_WIDTH, }, 1},
    {{ ARROW_WIDTH, LINE_END, -ARROW_WIDTH, }, 1},
    {{ -ARROW_WIDTH,LINE_END,ARROW_WIDTH, }, 1},
    {{ ARROW_WIDTH, LINE_END, ARROW_WIDTH, }, 1},
    {{ 0.0f, LENGTH, 0.0f, }, 1},
    // Z-axis arrow
    {{ -ARROW_WIDTH, -ARROW_WIDTH, LINE_END, }, 2},
    {{ ARROW_WIDTH, -ARROW_WIDTH, LINE_END, }, 2},
    {{ -ARROW_WIDTH, ARROW_WIDTH, LINE_END, }, 2},
    {{ ARROW_WIDTH, ARROW_WIDTH, LINE_END, }, 2},
    {{ 0.0f, 0.0f, LENGTH, }, 2},
};
// clang-format on

constexpr uint8_t XY_IDX_OFFSET      = 0;
constexpr uint8_t XZ_IDX_OFFSET      = 4;
constexpr uint8_t YX_IDX_OFFSET      = 8;
constexpr uint8_t YZ_IDX_OFFSET      = 12;
constexpr uint8_t ZX_IDX_OFFSET      = 16;
constexpr uint8_t ZY_IDX_OFFSET      = 20;
constexpr uint8_t X_ARROW_IDX_OFFSET = 24;
constexpr uint8_t Y_ARROW_IDX_OFFSET = 29;
constexpr uint8_t Z_ARROW_IDX_OFFSET = 34;

constexpr uint8_t indices[] = {
    XY_IDX_OFFSET,          XY_IDX_OFFSET + 2,      XY_IDX_OFFSET + 1,      XY_IDX_OFFSET + 3,
    XY_IDX_OFFSET + 1,      XY_IDX_OFFSET + 2,      XZ_IDX_OFFSET,          XZ_IDX_OFFSET + 2,
    XZ_IDX_OFFSET + 1,      XZ_IDX_OFFSET + 3,      XZ_IDX_OFFSET + 1,      XZ_IDX_OFFSET + 2,
    YX_IDX_OFFSET,          YX_IDX_OFFSET + 2,      YX_IDX_OFFSET + 1,      YX_IDX_OFFSET + 3,
    YX_IDX_OFFSET + 1,      YX_IDX_OFFSET + 2,      YZ_IDX_OFFSET,          YZ_IDX_OFFSET + 2,
    YZ_IDX_OFFSET + 1,      YZ_IDX_OFFSET + 3,      YZ_IDX_OFFSET + 1,      YZ_IDX_OFFSET + 2,
    ZX_IDX_OFFSET,          ZX_IDX_OFFSET + 2,      ZX_IDX_OFFSET + 1,      ZX_IDX_OFFSET + 3,
    ZX_IDX_OFFSET + 1,      ZX_IDX_OFFSET + 2,      ZY_IDX_OFFSET,          ZY_IDX_OFFSET + 2,
    ZY_IDX_OFFSET + 1,      ZY_IDX_OFFSET + 3,      ZY_IDX_OFFSET + 1,      ZY_IDX_OFFSET + 2,
    X_ARROW_IDX_OFFSET,     X_ARROW_IDX_OFFSET + 1, X_ARROW_IDX_OFFSET + 4, X_ARROW_IDX_OFFSET,
    X_ARROW_IDX_OFFSET + 2, X_ARROW_IDX_OFFSET + 4, X_ARROW_IDX_OFFSET + 1, X_ARROW_IDX_OFFSET + 3,
    X_ARROW_IDX_OFFSET + 4, X_ARROW_IDX_OFFSET + 2, X_ARROW_IDX_OFFSET + 3, X_ARROW_IDX_OFFSET + 4,
    Y_ARROW_IDX_OFFSET,     Y_ARROW_IDX_OFFSET + 1, Y_ARROW_IDX_OFFSET + 4, Y_ARROW_IDX_OFFSET,
    Y_ARROW_IDX_OFFSET + 2, Y_ARROW_IDX_OFFSET + 4, Y_ARROW_IDX_OFFSET + 1, Y_ARROW_IDX_OFFSET + 3,
    Y_ARROW_IDX_OFFSET + 4, Y_ARROW_IDX_OFFSET + 2, Y_ARROW_IDX_OFFSET + 3, Y_ARROW_IDX_OFFSET + 4,
    Z_ARROW_IDX_OFFSET,     Z_ARROW_IDX_OFFSET + 1, Z_ARROW_IDX_OFFSET + 4, Z_ARROW_IDX_OFFSET,
    Z_ARROW_IDX_OFFSET + 2, Z_ARROW_IDX_OFFSET + 4, Z_ARROW_IDX_OFFSET + 1, Z_ARROW_IDX_OFFSET + 3,
    Z_ARROW_IDX_OFFSET + 4, Z_ARROW_IDX_OFFSET + 2, Z_ARROW_IDX_OFFSET + 3, Z_ARROW_IDX_OFFSET + 4,
};

constexpr GLint UNIFORM_MVP_LOCATION   = 0;
constexpr GLint UNIFORM_SCALE_LOCATION = 1;

} // namespace

namespace engine::gl {

ENGINE_EXPORT auto AxesRenderer::Allocate(GlContext& gl) -> AxesRenderer {
    constexpr GLint ATTRIB_POSITION_LOCATION = 0;
    constexpr GLint ATTRIB_COLOR_LOCATION    = 1;
    AxesRenderer renderer;
    renderer.attributeBuffer_ = gl::GpuBuffer::Allocate(
        gl, GL_ARRAY_BUFFER, {}, CpuMemory<GLvoid const>{vertexData, sizeof(vertexData)}, "AxesRenderer Vertices");
    renderer.indexBuffer_ = gl::GpuBuffer::Allocate(
        gl, GL_ELEMENT_ARRAY_BUFFER, {}, CpuMemory<GLvoid const>{indices, sizeof(indices)}, "AxesRenderer Indices");
    renderer.vao_ = gl::Vao::Allocate(gl, "AxesRenderer");
    std::ignore   = gl::VaoMutableCtx{renderer.vao_}
                      .MakeVertexAttribute(
                          renderer.attributeBuffer_,
                          {.location        = ATTRIB_POSITION_LOCATION,
                           .valuesPerVertex = 3,
                           .datatype        = GL_FLOAT,
                           .stride          = sizeof(Vertex),
                           .offset          = offsetof(Vertex, position)})
                      .MakeVertexAttribute(
                          renderer.attributeBuffer_,
                          {.location        = ATTRIB_COLOR_LOCATION,
                           .valuesPerVertex = 1,
                           .datatype        = GL_UNSIGNED_INT,
                           .stride          = sizeof(Vertex),
                           .offset          = offsetof(Vertex, colorIdx)})
                      .MakeIndexed(renderer.indexBuffer_, GL_UNSIGNED_BYTE);

    std::vector<ShaderDefine> defines = {
        ShaderDefine::I32("ATTRIB_POSITION", ATTRIB_POSITION_LOCATION),
        ShaderDefine::I32("ATTRIB_COLOR", ATTRIB_COLOR_LOCATION),
        ShaderDefine::I32("UNIFORM_MVP", UNIFORM_MVP_LOCATION),
        ShaderDefine::I32("UNIFORM_SCALE", UNIFORM_SCALE_LOCATION),
    };

    auto makeProgram = [&](std::shared_ptr<GpuProgram>& out, std::string_view name) {
        auto definesClone = defines;
        auto maybeProgram = gl::LinkProgramFromFiles(
            gl, "data/engine/shaders/axes.vert", "data/engine/shaders/color_palette.frag", std::move(definesClone),
            name);
        assert(maybeProgram);
        out = std::move(*maybeProgram);
    };

    makeProgram(renderer.customizedProgram_, "AxesRenderer");
    makeProgram(renderer.defaultProgram_, "AxesRenderer/Default");
    gl::UniformCtx{*renderer.defaultProgram_}.SetUniformValue3(UNIFORM_SCALE_LOCATION, 1.0f, 1.0f, 1.0f);

    return renderer;
}

ENGINE_EXPORT void AxesRenderer::Dispose(GlContext const& gl) {

}

ENGINE_EXPORT void AxesRenderer::Render(GlContext& gl, glm::mat4 const& mvp, float scale) const {
    bool isCustom            = scale != 1.0f;
    auto const& program = isCustom ? *customizedProgram_ : *defaultProgram_;

    auto programGuard = gl::UniformCtx{program};
    programGuard.SetUniformMatrix4x4(UNIFORM_MVP_LOCATION, glm::value_ptr(mvp));
    if (isCustom) { programGuard.SetUniformValue3(UNIFORM_SCALE_LOCATION, scale, scale, scale); }

    gl.RenderState().DepthTestWrite();
    RenderVao(vao_);
}

} // namespace engine::gl