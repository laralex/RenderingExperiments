
#include "engine/gl/FlatRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

struct UboData {
    alignas(16) glm::mat4 mvp;
    alignas(16) glm::mat4 modelToWorld;
    alignas(16) glm::mat3x4 normalToWorld;
    alignas(16) glm::vec4 ambientIntensity;
    alignas(16) glm::vec4 materialColor;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec4 lightWorldPosition;
};

constexpr static int32_t ATTRIB_POSITION_LOCATION = 0;
constexpr static int32_t ATTRIB_UV_LOCATION       = 1;
constexpr static int32_t ATTRIB_NORMAL_LOCATION   = 2;

constexpr GLint UBO_BINDING          = 5; // global for GL

} // namespace

namespace engine::gl {

auto FlatRenderer::Allocate() -> FlatRenderer {
    FlatRenderer renderer;

    gl::shader::Define const defines[] = {
        {.name = "ATTRIB_POSITION", .value = ATTRIB_POSITION_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "ATTRIB_UV", .value = ATTRIB_UV_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "ATTRIB_NORMAL", .value = ATTRIB_NORMAL_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "UBO_BINDING", .value = UBO_BINDING, .type = gl::shader::Define::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/lambert.vert", "data/engine/shaders/lambert.frag", CpuView{defines, std::size(defines)},
        "Lambert diffuse", false);
    assert(maybeProgram);
    renderer.program_ = std::move(*maybeProgram);

    renderer.ubo_ =
        gl::GpuBuffer::Allocate(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW,
        CpuMemory<void const>{nullptr, sizeof(UboData)}, "FlatRenderer UBO");
    renderer.uboLocation_ = UniformCtx::GetUboLocation(renderer.program_, "Ubo");

    return renderer;
}

void FlatRenderer::Render(FlatRenderArgs const& args) const {
    glm::mat3x4 normalToWorld = glm::transpose(glm::inverse(args.modelToWorld));
    // XLOGE("{} {} {} {}", normalToWorld4[0].x, normalToWorld4[0].y, normalToWorld4[0].z, normalToWorld4[0].w);
    // XLOGE("{} {} {} {}", normalToWorld4[1].x, normalToWorld4[1].y, normalToWorld4[1].z, normalToWorld4[1].w);
    // XLOGE("{} {} {} {}", normalToWorld4[2].x, normalToWorld4[2].y, normalToWorld4[2].z, normalToWorld4[2].w);
    // XLOGE("{} {} {} {}", normalToWorld4[3].x, normalToWorld4[3].y, normalToWorld4[3].z, normalToWorld4[3].w);
    // glm::mat3 normalToWorld{normalToWorld4[0], normalToWorld4[1], normalToWorld4[2]};
    // XLOGE("{} {} {}", normalToWorld[0].x, normalToWorld[0].y, normalToWorld[0].z);
    // XLOGE("{} {} {}", normalToWorld[1].x, normalToWorld[1].y, normalToWorld[1].z);
    // XLOGE("{} {} {}", normalToWorld[2].x, normalToWorld[2].y, normalToWorld[2].z);
    UboData data {
        .mvp = args.mvp,
        .modelToWorld = args.modelToWorld,
        .normalToWorld = normalToWorld,
        .ambientIntensity      = glm::vec4{0.01, 0.01, 0.01, 1.0},
        .materialColor         = glm::vec4{args.materialColor, 1.0},
        .lightColor            = glm::vec4{args.lightColor, 1.0},
        .lightWorldPosition = args.lightWorldPosition,
    };

    ubo_.Fill(CpuMemory<GLvoid const>{&data, sizeof(data)});
    GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BINDING, ubo_.Id()));

    auto programGuard = gl::UniformCtx(program_);
    RenderVao(args.vaoWithNormal, args.primitive);
}

} // namespace engine::gl