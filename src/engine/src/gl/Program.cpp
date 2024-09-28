#include "engine/gl/Program.hpp"

#include "engine_private/Prelude.hpp"
#include <optional>

namespace {

constexpr bool LOG_FAILED_SHADER_CODE = true;

} // namespace

namespace engine::gl {

ENGINE_EXPORT void GpuProgram::Dispose() {
    if (programId_ == GL_NONE) { return; }
    // LogDebugLabel(*this, "GpuProgram was disposed");
    XLOG("GpuProgram was disposed: 0x{:08X}", GLuint(programId_));
    GLCALL(glDeleteProgram(programId_));
    programId_.UnsafeReset();
}

ENGINE_EXPORT auto GpuProgram::LinkGraphical(GLuint vertexShader, GLuint fragmentShader) const -> bool {
    GLCALL(glAttachShader(programId_, vertexShader));
    GLCALL(glAttachShader(programId_, fragmentShader));

    GLCALL(glLinkProgram(programId_));
    GLint isLinked;
    GLCALL(glGetProgramiv(programId_, GL_LINK_STATUS, &isLinked));

    GLCALL(glDetachShader(programId_, vertexShader));
    GLCALL(glDetachShader(programId_, fragmentShader));

    if (isLinked == GL_TRUE) { return true; }

    static char infoLog[512];
    GLCALL(glGetProgramInfoLog(programId_, 512, nullptr, infoLog));
    XLOGE("Failed to link graphics program:\n{}", infoLog);
    return false;
}

ENGINE_EXPORT auto GpuProgram::Allocate(
    GlContext const& gl, GLuint vertexShader, GLuint fragmentShader, std::string_view name)
    -> std::optional<GpuProgram> {
    auto program = GpuProgram();
    GLuint programId;
    GLCALL(programId = glCreateProgram());
    program.programId_ = programId;

    if (!program.LinkGraphical(vertexShader, fragmentShader)) {
        XLOGE("Failed to link graphics program name={}", name);
        GLCALL(glDeleteProgram(program.programId_));
        return std::nullopt;
    }

    if (!name.empty()) {
        DebugLabel(gl, program, name);
        LogDebugLabel(gl, program, "GpuProgram was compiled");
    }
    return std::optional{std::move(program)};
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
        // XLOG("Compiled shader {}", shader);
        return shader; // success
    }

    static char infoLog[512];
    GLCALL(glGetShaderInfoLog(shader, 512, nullptr, infoLog));
    GLCALL(glDeleteShader(shader));
    std::string_view typeLabel =
        (shaderType == GL_VERTEX_SHADER ? "vertex" : (shaderType == GL_FRAGMENT_SHADER ? "fragment" : "compute"));
    XLOGE("Failed to compile {} shader:\n{}\n{}", typeLabel, LOG_FAILED_SHADER_CODE ? code : "", infoLog);
    return GL_NONE;
}

} // namespace engine::gl