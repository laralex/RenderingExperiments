#include "engine/GlCapabilities.hpp"

namespace engine::gl {

bool GlCapabilities::isInitialized    = false;
GLint GlCapabilities::maxTextureUnits = 0xDEAD;
GLint GlCapabilities::numExtensions   = 0xDEAD;
GLint GlCapabilities::majorVersion    = 0xDEAD;
GLint GlCapabilities::minorVersion    = 0xDEAD;

void GlCapabilities::Initialize() {
    if (isInitialized) { return; }
#define GET_GL_CAP(cap, variable) \
        GLCALL(glGetIntegerv(cap, &variable)); \
        XLOG("GlCapabilities::" #cap " = {}", variable);

    GET_GL_CAP(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, maxTextureUnits);
    GET_GL_CAP(GL_NUM_EXTENSIONS, numExtensions);
    GET_GL_CAP(GL_MAJOR_VERSION, majorVersion);
    GET_GL_CAP(GL_MINOR_VERSION, minorVersion);
#undef GET_GL_CAP
    isInitialized = true;
}

} // namespace engine::gl