#include "engine/Prelude.hpp"

namespace engine {

void CheckOpenGLError(const char* stmt, const char* fname, int line, bool fatal) {
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) { return; }
    char const* errLabel = "UNKNOWN_ERROR";
    switch (err) {
    case GL_INVALID_ENUM:
        errLabel = "GL_INVALID_ENUM";
        break;
    case GL_INVALID_VALUE:
        errLabel = "GL_INVALID_VALUE";
        break;
    case GL_INVALID_OPERATION:
        errLabel = "GL_INVALID_OPERATION";
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        errLabel = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
    }
    XLOGE("{}(0x{:x}), at {}:{} - for {}\n", errLabel, err, fname, line, stmt);
    if (fatal) { std::terminate(); }
}

} // namespace engine
