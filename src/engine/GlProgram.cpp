#include "engine/GlProgram.hpp"
#include "engine_private/Prelude.hpp"

namespace engine {

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

ENGINE_EXPORT auto CompileGraphicsProgram(GLuint vertexShader, GLuint fragmentShader) -> GLuint {
    GLenum program;
    GLCALL(program = glCreateProgram());
    GLCALL(glAttachShader(program, vertexShader));
    GLCALL(glAttachShader(program, fragmentShader));

    GLCALL(glLinkProgram(program));
    GLint isLinked;
    GLCALL(glGetProgramiv(program, GL_LINK_STATUS, &isLinked));

    GLCALL(glDetachShader(program, vertexShader));
    GLCALL(glDetachShader(program, fragmentShader));

    if (isLinked == GL_TRUE) {
        XLOG("Linked graphics program {} (v={}, f={})", program, vertexShader, fragmentShader);
        return program; // success
    }

    static char infoLog[512];
    GLCALL(glGetProgramInfoLog(program, 512, nullptr, infoLog));
    GLCALL(glDeleteProgram(program));
    XLOGE("Failed to link graphics program:\n{}", infoLog);

    return GL_NONE;
}

} // namespace engine