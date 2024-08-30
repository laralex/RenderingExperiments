#include "engine/gl/Capabilities.hpp"

namespace engine::gl {

bool GlCapabilities::isInitialized    = false;
GLint GlCapabilities::maxTextureUnits = 0xDEAD;
GLint GlCapabilities::numExtensions   = 0xDEAD;
GLint GlCapabilities::majorVersion    = 0xDEAD;
GLint GlCapabilities::minorVersion    = 0xDEAD;
GLint GlCapabilities::maxUboBindings  = 0xDEAD;
GLint GlCapabilities::maxUboBlocksVertex  = 0xDEAD;
GLint GlCapabilities::maxUboBlocksFragment  = 0xDEAD;
GLint GlCapabilities::maxUboBlocksGeometry  = 0xDEAD;
GLint GlCapabilities::uboOffsetAlignment  = 0xDEAD;

void GlCapabilities::Initialize() {
    if (isInitialized) { return; }

    auto getCapability = [](GLenum cap, char const* capStr, GLint& destination) {
        GLCALL(glGetIntegerv(cap, &destination));
        XLOG("GlCapabilities::{} = {}", capStr, destination);
    };

    getCapability(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS", maxTextureUnits);
    getCapability(GL_NUM_EXTENSIONS, "GL_NUM_EXTENSIONS", numExtensions);
    getCapability(GL_MAJOR_VERSION, "GL_MAJOR_VERSION", majorVersion);
    getCapability(GL_MINOR_VERSION, "GL_MINOR_VERSION", minorVersion);
    getCapability(GL_MAX_UNIFORM_BUFFER_BINDINGS, "GL_MAX_UNIFORM_BUFFER_BINDINGS", maxUboBindings);
    getCapability(GL_MAX_VERTEX_UNIFORM_BLOCKS, "GL_MAX_VERTEX_UNIFORM_BLOCKS", maxUboBlocksVertex);
    getCapability(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, "GL_MAX_FRAGMENT_UNIFORM_BLOCKS", maxUboBlocksFragment);
    getCapability(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, "GL_MAX_GEOMETRY_UNIFORM_BLOCKS", maxUboBlocksGeometry);
    getCapability(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, "GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT", uboOffsetAlignment);

    isInitialized = true;
}

} // namespace engine::gl