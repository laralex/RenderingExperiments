#include "engine/gl/GpuProgram.hpp"

#include "engine_private/Prelude.hpp"
#include <optional>

namespace {

auto LinkGraphicalProgram(GLuint program, GLuint vertexShader, GLuint fragmentShader) -> bool {
    GLCALL(glAttachShader(program, vertexShader));
    GLCALL(glAttachShader(program, fragmentShader));

    GLCALL(glLinkProgram(program));
    GLint isLinked;
    GLCALL(glGetProgramiv(program, GL_LINK_STATUS, &isLinked));

    GLCALL(glDetachShader(program, vertexShader));
    GLCALL(glDetachShader(program, fragmentShader));

    if (isLinked == GL_TRUE) { return true; }

    static char infoLog[512];
    GLCALL(glGetProgramInfoLog(program, 512, nullptr, infoLog));
    XLOGE("Failed to link graphics program:\n{}", infoLog);
    return false;
}

} // namespace

namespace engine::gl {

ENGINE_EXPORT void GpuProgram::Dispose() {
    if (programId_ == GL_NONE) { return; }
    // LogDebugLabel(*this, "GpuProgram was disposed");
    XLOG("GpuProgram was disposed: 0x{:08X}", GLuint(programId_));
    GLCALL(glDeleteProgram(programId_));
    programId_.UnsafeReset();
}

ENGINE_EXPORT auto GpuProgram::LinkGraphical(GLuint vertexShader, GLuint fragmentShader, bool isRecompile) const
    -> bool {
    if (!isRecompile) {
        // first compilation
        return LinkGraphicalProgram(programId_, vertexShader, fragmentShader);
    }
    // attempt to hot-reload
    // compile into temporary program, if it fails, don't recompile the existing program
    GLuint tmpProgramId;
    GLCALL(tmpProgramId = glCreateProgram());
    bool ok = LinkGraphicalProgram(tmpProgramId, vertexShader, fragmentShader);
    GLCALL(glDeleteProgram(tmpProgramId));
    if (ok) { ok = LinkGraphicalProgram(programId_, vertexShader, fragmentShader); }
    return ok;
}

ENGINE_EXPORT auto GpuProgram::Allocate(
    GlContext& gl, GLuint vertexShader, GLuint fragmentShader, std::string_view name)
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

} // namespace engine::gl