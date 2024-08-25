#include "engine/Assets.hpp"
#include "engine/Prelude.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Vao.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

auto LinkProgram(std::string_view vertexShaderCode, std::string_view fragmentShaderCode, std::string_view name)
    -> std::optional<GpuProgram> {
    GLuint vertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderCode);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);

    auto maybeProgram = GpuProgram::Allocate(vertexShader, fragmentShader, name);
    assert(maybeProgram);

    GLCALL(glDeleteShader(vertexShader));
    GLCALL(glDeleteShader(fragmentShader));

    return maybeProgram;
}

auto LinkProgramFromFiles(
    std::string_view vertexFilepath, std::string_view fragmentFilepath, CpuView<ShaderDefine const> defines,
    std::string_view name) -> std::optional<GpuProgram> {
    std::string vertexShaderCode   = LoadShaderCode(vertexFilepath, defines);
    std::string fragmentShaderCode = LoadShaderCode(fragmentFilepath, defines);
    return LinkProgram(vertexShaderCode, fragmentShaderCode, name);
}

void RenderVao(Vao const& vao, GLenum primitive) {
    auto vaoGuard      = VaoCtx{vao};
    GLint firstIndex   = vao.FirstIndex();
    GLsizei numIndices = vao.IndexCount();
    if (vao.IsIndexed()) {
        auto const* firstIndexPtr = reinterpret_cast<decltype(firstIndex) const*>(firstIndex);
        GLCALL(glDrawElements(primitive, numIndices, vao.IndexDataType(), firstIndexPtr));
    } else {
        GLCALL(glDrawArrays(primitive, firstIndex, numIndices));
    }
}

auto TransformOrigin(glm::mat4& transform, bool isRowMajor) -> glm::vec3 {
    if (isRowMajor) { return glm::vec3{transform[0][3], transform[1][3], transform[2][3]}; }
    return glm::vec3{transform[3][0], transform[3][1], transform[3][2]};
}

void UndoAffineScale(glm::mat4& transform) {
    auto invThenTransposed = glm::inverse(transform);
    invThenTransposed      = glm::transpose(invThenTransposed);
    transform              = invThenTransposed * transform;
}

auto UndoAffineScale(glm::mat4 const& transform) -> glm::mat4 {
    auto invThenTransposed = glm::inverse(transform);
    invThenTransposed      = glm::transpose(invThenTransposed);
    return invThenTransposed * transform;
}

} // namespace engine::gl