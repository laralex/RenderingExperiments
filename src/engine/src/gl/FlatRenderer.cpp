
#include "engine/gl/FlatRenderer.hpp"
#include "engine/Assets.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Uniform.hpp"

namespace {

constexpr GLint UNIFORM_MODEL_LOCATION    = 0;
constexpr GLint UNIFORM_VIEWPROJ_LOCATION = 1;
constexpr GLint UBO_CONTEXT_BINDING       = 0; // global for GL
constexpr GLint UBO_SHADER_BINDING        = 0; // local for shader

struct UboData {
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec4 lightPosition;
};

} // namespace

namespace engine::gl {

auto FlatRenderer::Allocate() -> FlatRenderer {
    FlatRenderer renderer;

    constexpr static int32_t ATTRIB_POSITION_LOCATION = 0;
    constexpr static int32_t ATTRIB_UV_LOCATION       = 1;
    constexpr static int32_t ATTRIB_NORMAL_LOCATION   = 2;
    gl::ShaderDefine const defines[]                  = {
        {.name = "ATTRIB_POSITION", .value = ATTRIB_POSITION_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_UV", .value = ATTRIB_UV_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_NORMAL", .value = ATTRIB_NORMAL_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_MODEL", .value = UNIFORM_MODEL_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_VIEWPROJ", .value = UNIFORM_VIEWPROJ_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UBO_BINDING", .value = UBO_SHADER_BINDING, .type = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/engine/shaders/lambert.vert", "data/engine/shaders/lambert.frag", CpuView{defines, std::size(defines)},
        "Lambert diffuse");
    assert(maybeProgram);
    renderer.program_ = std::move(*maybeProgram);

    renderer.ubo_ =
        gl::GpuBuffer::Allocate(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, nullptr, sizeof(UboData), "FlatRenderer UBO");

    return renderer;
}

void FlatRenderer::Render(
    Vao const& vaoWithNormal, GLenum primitive, glm::mat4 const& model, glm::mat4 const& camera,
    glm::vec3 lightPosition) const {

    UboData data{
        .lightColor    = glm::vec4{0.3, 1.0, 0.1, 1.0},
        .lightPosition = glm::vec4{lightPosition, 0.0},
    };

    ubo_.Fill(&data, sizeof(data));
    GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_CONTEXT_BINDING, ubo_.Id()));

    auto programGuard = gl::UniformCtx(program_);
    programGuard.SetUbo(UBO_SHADER_BINDING, UBO_CONTEXT_BINDING);

    programGuard.SetUniformMatrix4(UNIFORM_MODEL_LOCATION, glm::value_ptr(model));
    programGuard.SetUniformMatrix4(UNIFORM_VIEWPROJ_LOCATION, glm::value_ptr(camera));

    RenderVao(vaoWithNormal, primitive);
}

} // namespace engine::gl