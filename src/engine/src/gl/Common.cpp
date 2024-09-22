#include "engine/Assets.hpp"
#include "engine/Precompiled.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Vao.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

auto LinkProgram(
    GlContext const& gl, std::string_view vertexShaderCode, std::string_view fragmentShaderCode, std::string_view name,
    bool logCode) -> std::optional<GpuProgram> {
    if (logCode) {
        XLOG("Compiling program [{}] type=vertex\n{}", name, vertexShaderCode);
        XLOG("Compiling name [{}] type=fragment\n{}", name, fragmentShaderCode);
    }
    GLuint vertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderCode);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);

    auto maybeProgram = GpuProgram::Allocate(gl, vertexShader, fragmentShader, name);
    assert(maybeProgram);

    GLCALL(glDeleteShader(vertexShader));
    GLCALL(glDeleteShader(fragmentShader));

    return maybeProgram;
}

auto LinkProgramFromFiles(
    GlContext const& gl, std::string_view vertexFilepath, std::string_view fragmentFilepath,
    CpuView<shader::Define const> defines, std::string_view name, bool logCode) -> std::optional<GpuProgram> {
    std::string vertexShaderCode   = LoadShaderCode(vertexFilepath, shader::ShaderType::VERTEX, defines);
    std::string fragmentShaderCode = LoadShaderCode(fragmentFilepath, shader::ShaderType::FRAGMENT, defines);
    return LinkProgram(gl, vertexShaderCode, fragmentShaderCode, name, logCode);
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

void RenderVaoInstanced(Vao const& vao, GLuint firstInstance, GLsizei numInstances, GLenum primitive) {
    auto vaoGuard      = VaoCtx{vao};
    GLint firstIndex   = vao.FirstIndex();
    GLsizei numIndices = vao.IndexCount();
    if (vao.IsIndexed()) {
        auto const* firstIndexPtr = reinterpret_cast<decltype(firstIndex) const*>(firstIndex);
        GLCALL(glDrawElementsInstancedBaseInstance(
            primitive, numIndices, vao.IndexDataType(), firstIndexPtr, numInstances, firstInstance));
    } else {
        GLCALL(glDrawArraysInstancedBaseInstance(primitive, firstIndex, numIndices, numInstances, firstInstance));
    }
}

auto TransformOrigin(glm::mat4 const& transform, bool isRowMajor) -> glm::vec3 {
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