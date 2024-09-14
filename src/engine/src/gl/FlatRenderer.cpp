
#include "engine/gl/FlatRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

struct LightCoefficients final {
    float radius = 1.0f;
    float __pad0 = 0.0f;
    float __pad1 = 0.0f;
    float __pad2 = 0.0f;
};

struct LightData final {
    alignas(16) glm::vec4 worldPosition;
    alignas(16) glm::vec4 diffuseColor;
    alignas(16) glm::vec4 ambientIntensity;
    alignas(16) LightCoefficients coefficients;
};

struct MaterialCoefficients final {
    float specularity = 1.0f;
    float specularPower = 32.0f;
    float __pad0 = 0.0f;
    float __pad1 = 0.0f;
};

struct Material final {
    alignas(16) glm::vec4 diffuseColor{1.0f};
    alignas(16) MaterialCoefficients coefficients{};
};

struct UboData final {
    alignas(16) glm::mat4 mvp{};
    alignas(16) glm::mat4 modelToWorld{};
    alignas(16) glm::mat3x4 normalToWorld{};
    alignas(16) glm::vec4 eyeWorldDirection{};
    alignas(16) Material material{};
    alignas(16) LightData lights[1]{};
};

constexpr static int32_t ATTRIB_POSITION_LOCATION = 0;
constexpr static int32_t ATTRIB_UV_LOCATION       = 1;
constexpr static int32_t ATTRIB_NORMAL_LOCATION   = 2;

constexpr GLint UBO_BINDING = 5; // global for GL

} // namespace

namespace engine::gl {

auto FlatRenderer::Allocate() -> FlatRenderer {
    FlatRenderer renderer;

    gl::shader::Define const defines[] = {
        {.name = "ATTRIB_POSITION", .value = ATTRIB_POSITION_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "ATTRIB_UV", .value = ATTRIB_UV_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "ATTRIB_NORMAL", .value = ATTRIB_NORMAL_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "UBO_BINDING", .value = UBO_BINDING, .type = gl::shader::Define::INT32},
        {.name = "USE_SPECULAR", .value = true, .type = gl::shader::Define::BOOLEAN8},
        {.name = "USE_PHONG", .value = false, .type = gl::shader::Define::BOOLEAN8},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/blinn_phong.vert", "data/engine/shaders/blinn_phong.frag", CpuView{defines, std::size(defines)},
        "Lambert diffuse", false);
    assert(maybeProgram);
    renderer.program_ = std::move(*maybeProgram);

    renderer.ubo_ = gl::GpuBuffer::Allocate(
        GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, CpuMemory<void const>{nullptr, sizeof(UboData)}, "FlatRenderer UBO");
    renderer.uboLocation_ = UniformCtx::GetUboLocation(renderer.program_, "Ubo");

    return renderer;
}

void FlatRenderer::Render(FlatRenderArgs const& args) const {
    glm::mat3x4 normalToWorld = glm::transpose(glm::inverse(args.modelToWorld));

    UboData data{
        .mvp                 = args.mvp,
        .modelToWorld        = args.modelToWorld,
        .normalToWorld       = normalToWorld,
        .eyeWorldDirection   = glm::vec4{glm::normalize(args.eyeWorldPosition), 0.0f},
        .material      = {
            .diffuseColor = glm::vec4{args.materialColor, 1.0f},
            .coefficients = {
                .specularity = args.materialSpecularIntensity,
                .specularPower = args.materialSpecularPower,
            },
        },
        .lights = { {
            .worldPosition  = glm::vec4{args.lightWorldPosition, 1.0f},
            .diffuseColor = glm::vec4{args.lightColor, 1.0f},
            .ambientIntensity    = glm::vec4{0.01f, 0.01f, 0.01f, 1.0f},
            .coefficients = {
                .radius = 1.0f,
            }
        } }
    };

    ubo_.Fill(CpuMemory<GLvoid const>{&data, sizeof(data)});
    GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BINDING, ubo_.Id()));

    auto programGuard = gl::UniformCtx(program_);
    RenderVao(args.vaoWithNormal, args.primitive);
}

} // namespace engine::gl