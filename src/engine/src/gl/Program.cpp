#include "engine/gl/Program.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_EXPORT void GpuProgram::Dispose() {
    if (programId_ == GL_NONE) { return; }
    LogDebugLabel(*this, "GpuProgram was disposed");
    GLCALL(glDeleteProgram(programId_));
    programId_.id_ = GL_NONE;
}

ENGINE_EXPORT auto GpuProgram::Allocate(GLuint vertexShader, GLuint fragmentShader, std::string_view name)
    -> std::optional<GpuProgram> {
    GLuint programId;
    GLCALL(programId = glCreateProgram());
    GLCALL(glAttachShader(programId, vertexShader));
    GLCALL(glAttachShader(programId, fragmentShader));

    GLCALL(glLinkProgram(programId));
    GLint isLinked;
    GLCALL(glGetProgramiv(programId, GL_LINK_STATUS, &isLinked));

    GLCALL(glDetachShader(programId, vertexShader));
    GLCALL(glDetachShader(programId, fragmentShader));

    if (isLinked == GL_TRUE) {
        auto program          = GpuProgram();
        program.programId_.id_ = programId;
        if (!name.empty()) {
            DebugLabel(program, name);
            LogDebugLabel(program, "GpuProgram was compiled");
        }
        return std::optional{std::move(program)}; // success
    }

    static char infoLog[512];
    GLCALL(glGetProgramInfoLog(programId, 512, nullptr, infoLog));
    GLCALL(glDeleteProgram(programId));
    XLOGE("Failed to link graphics program (name={}):\n{}", name, infoLog);

    return std::nullopt;
}

ENGINE_EXPORT auto CompileShader(GLenum shaderType, std::string_view code) -> GLuint {
    GLenum shader;
    GLCALL(shader = glCreateShader(shaderType));

    char const* codeRaw  = code.data();
    GLint const codeSize = code.size();
    GLCALL(glShaderSource(shader, 1, &codeRaw, &codeSize));

    GLCALL(glCompileShader(shader));

    GLint isCompiled;
    GLCALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled));
    if (isCompiled == GL_TRUE) {
        XLOG("Compiled shader {}", shader);
        return shader; // success
    }

    static char infoLog[512];
    GLCALL(glGetShaderInfoLog(shader, 512, nullptr, infoLog));
    GLCALL(glDeleteShader(shader));
    char typeLabel = (shaderType == GL_VERTEX_SHADER ? 'v' : (shaderType == GL_FRAGMENT_SHADER ? 'f' : 'c'));
    XLOGE("Failed to compile shader (type={}):\n{}", typeLabel, infoLog);

    return GL_NONE;
}

} // namespace engine::gl