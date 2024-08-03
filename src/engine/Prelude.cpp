#include "engine/Prelude.hpp"

namespace engine {
    
void CheckOpenGLError(const char* stmt, const char* fname, int line, bool fatal) {
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) { return; }
    XLOGE("GL error {:x}, at {}:{} - for {}\n", err, fname, line, stmt);
    if (fatal) { std::terminate(); }
}

} // namespace engine
