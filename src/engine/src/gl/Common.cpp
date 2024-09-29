#include "engine/Assets.hpp"
#include "engine/Precompiled.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Vao.hpp"

#include "engine_private/Prelude.hpp"

namespace {

void LogShaderCode(std::string_view vertexShaderCode, std::string_view fragmentShaderCode, bool doLog) {
    if (doLog) {
        XLOG("Compiling shader type=vertex\n{}", vertexShaderCode);
        XLOG("Compiling shader type=fragment\n{}", fragmentShaderCode);
    }
}

auto AllocateGraphicalShaders [[nodiscard]](std::string_view vertexShaderCode, std::string_view fragmentShaderCode, bool logCode) -> std::pair<GLuint, GLuint> {
    LogShaderCode(vertexShaderCode, fragmentShaderCode, logCode);
    GLuint vertexShader   = engine::gl::CompileShader(GL_VERTEX_SHADER, vertexShaderCode);
    GLuint fragmentShader = engine::gl::CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
    return {vertexShader, fragmentShader};
}

} // namespace anonymous

namespace engine::gl {

ENGINE_EXPORT auto LinkProgram(
    GlContext const& gl, std::string_view vertexShaderCode, std::string_view fragmentShaderCode, std::string_view name,
    bool logCode) -> std::optional<GpuProgram> {

    auto [vertexShader, fragmentShader] = AllocateGraphicalShaders(vertexShaderCode, fragmentShaderCode, logCode);
    auto maybeProgram = GpuProgram::Allocate(gl, vertexShader, fragmentShader, name);

    GLCALL(glDeleteShader(vertexShader));
    GLCALL(glDeleteShader(fragmentShader));

    return maybeProgram;
}

ENGINE_EXPORT auto LinkProgramFromFiles(
    GlContext const& gl, std::string_view vertexFilepath, std::string_view fragmentFilepath,
    CpuView<shader::Define const> defines, std::string_view name, bool logCode) -> std::optional<GpuProgram> {
    std::string vertexShaderCode   = LoadShaderCode(vertexFilepath, shader::ShaderType::VERTEX, defines);
    std::string fragmentShaderCode = LoadShaderCode(fragmentFilepath, shader::ShaderType::FRAGMENT, defines);
    return LinkProgram(gl, vertexShaderCode, fragmentShaderCode, name, logCode);
}

ENGINE_EXPORT auto RelinkProgram(
    GlContext const& gl, std::string_view vertexShaderCode, std::string_view fragmentShaderCode, GpuProgram const& oldProgram, bool logCode) -> bool {
    auto [vertexShader, fragmentShader] = AllocateGraphicalShaders(vertexShaderCode, fragmentShaderCode, logCode);
    // TODO: also fail if any shader didn't compile
    if (vertexShader <= 0 || fragmentShader <= 0) {
        GLCALL(glDeleteShader(vertexShader));
        GLCALL(glDeleteShader(fragmentShader));
        return false;
    }
    constexpr bool isRecompile = true;
    bool ok = oldProgram.LinkGraphical(vertexShader, fragmentShader, isRecompile);
    GLCALL(glDeleteShader(vertexShader));
    GLCALL(glDeleteShader(fragmentShader));
    return ok;
}

ENGINE_EXPORT auto RelinkProgramFromFiles(
    GlContext const& gl, std::string_view vertexFilepath, std::string_view fragmentFilepath,
    CpuView<shader::Define const> defines, GpuProgram const& oldProgram, bool logCode) -> bool {
    std::string vertexShaderCode   = LoadShaderCode(vertexFilepath, shader::ShaderType::VERTEX, defines);
    std::string fragmentShaderCode = LoadShaderCode(fragmentFilepath, shader::ShaderType::FRAGMENT, defines);
    return RelinkProgram(gl, vertexShaderCode, fragmentShaderCode, oldProgram, logCode);
}

ENGINE_EXPORT void RenderVao(Vao const& vao, GLenum primitive) {
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

ENGINE_EXPORT void RenderVaoInstanced(Vao const& vao, GLuint firstInstance, GLsizei numInstances, GLenum primitive) {
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

ENGINE_EXPORT auto TransformOrigin(glm::mat4 const& transform, bool isRowMajor) -> glm::vec3 {
    if (isRowMajor) { return glm::vec3{transform[0][3], transform[1][3], transform[2][3]}; }
    return glm::vec3{transform[3][0], transform[3][1], transform[3][2]};
}

ENGINE_EXPORT void UndoAffineScale(glm::mat4& transform) {
    auto invThenTransposed = glm::inverse(transform);
    invThenTransposed      = glm::transpose(invThenTransposed);
    transform              = invThenTransposed * transform;
}

ENGINE_EXPORT auto UndoAffineScale(glm::mat4 const& transform) -> glm::mat4 {
    auto invThenTransposed = glm::inverse(transform);
    invThenTransposed      = glm::transpose(invThenTransposed);
    return invThenTransposed * transform;
}

} // namespace engine::gl