#include "engine/gl/Capabilities.hpp"
#include "engine/Precompiled.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_EXPORT void GlCapabilities::Initialize() {
    if (isInitialized_) { return; }

    GLubyte const* vendor;
    GLCALL(vendor = glGetString(GL_VENDOR));
    vendor_ = std::string_view{reinterpret_cast<char const*>(vendor)};
    GLubyte const* renderer;
    GLCALL(renderer = glGetString(GL_RENDERER));
    renderer_ = std::string_view{reinterpret_cast<char const*>(renderer)};

    // if constexpr(XVERBOSE_BUILD) {
    XLOG("GlCapabilities::Vendor = {}", vendor_);
    XLOG("GlCapabilities::VendorDevice = {}", renderer_);
    // }

    auto getCapability = [](GLenum cap, std::string_view capStr, GLint& destination) {
        GLCALL(glGetIntegerv(cap, &destination));
        XLOGD("GlCapabilities::{} = {}", capStr, destination);
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
    getCapability(GL_MAX_DRAW_BUFFERS, "GL_MAX_DRAW_BUFFERS", maxDrawBuffers);

    isInitialized_ = true;
}

} // namespace engine::gl