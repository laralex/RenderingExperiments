
#include "engine/gl/FlatRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

struct UboData {
    alignas(16) glm::mat4 mvp;
    alignas(16) glm::vec4 ambientColor;
    alignas(16) glm::vec4 materialColor;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec4 lightTowardsDirection;
};

constexpr static int32_t ATTRIB_POSITION_LOCATION = 0;
constexpr static int32_t ATTRIB_UV_LOCATION       = 1;
constexpr static int32_t ATTRIB_NORMAL_LOCATION   = 2;

constexpr GLint UBO_BINDING          = 5; // global for GL

} // namespace

namespace engine::gl {

auto FlatRenderer::Allocate() -> FlatRenderer {
    FlatRenderer renderer;

    gl::ShaderDefine const defines[] = {
        {.name = "ATTRIB_POSITION", .value = ATTRIB_POSITION_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_UV", .value = ATTRIB_UV_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_NORMAL", .value = ATTRIB_NORMAL_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UBO_BINDING", .value = UBO_BINDING, .type = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/lambert.vert", "data/engine/shaders/lambert.frag", CpuView{defines, std::size(defines)},
        "Lambert diffuse");
    assert(maybeProgram);
    renderer.program_ = std::move(*maybeProgram);

    renderer.ubo_ =
        gl::GpuBuffer::Allocate(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW,
        CpuMemory<void const>{nullptr, sizeof(UboData)}, "FlatRenderer UBO");
    renderer.uboLocation_ = UniformCtx::GetUboLocation(renderer.program_, "Ubo");

    return renderer;
}

void FlatRenderer::Render(FlatRenderArgs const& args) const {
    // direction towards light in model space
    glm::vec4 towardsLight = args.invModel * glm::vec4{args.lightWorldPosition, 1.0f};
    towardsLight /= towardsLight.w;
    // towardsLight = glm::normalize(towardsLight);

    UboData data{
        .mvp = args.mvp,
        .ambientColor          = glm::vec4{0.1, 0.1, 0.1, 1.0},
        .materialColor         = glm::vec4{args.materialColor, 1.0},
        .lightColor            = glm::vec4{args.lightColor, 1.0},
        .lightTowardsDirection = towardsLight,
    };

    ubo_.Fill(CpuMemory<GLvoid const>{&data, sizeof(data)});
    GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BINDING, ubo_.Id()));

    auto programGuard = gl::UniformCtx(program_);
    RenderVao(args.vaoWithNormal, args.primitive);
}

} // namespace engine::gl