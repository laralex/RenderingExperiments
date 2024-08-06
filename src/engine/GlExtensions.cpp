#include "engine/GlExtensions.hpp"
#include "engine/GlCapabilities.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

bool GlExtensions::isInitialized = false;
std::unordered_set<std::string> GlExtensions::allExtensions{};
bool GlExtensions::hardcodedExtensions[static_cast<size_t>(Name::NUM_HARDCODED_EXTENSIONS)]{};

void GlExtensions::Initialize() {
    if (isInitialized) { return; }
    GLint numExtensions;
    GLCALL(glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions));
    for (GLint i = 0; i < numExtensions; ++i) {
        GLubyte const* ext = glGetStringi(GL_EXTENSIONS, i);
        auto extensionName = std::string{reinterpret_cast<char const*>(ext)};
        // XLOG("Extension {}", extensionName);
        allExtensions.insert(extensionName);
    }
    auto supports = [](char const* ext) {
        bool supported = allExtensions.find(ext) != allExtensions.end();
        XLOG("Extension {} supported={}", ext, static_cast<int>(supported));
        return supported;
    };
    hardcodedExtensions[KHR_debug]                              = supports("GL_KHR_debug");
    hardcodedExtensions[ARB_framebuffer_sRGB]                   = supports("GL_ARB_framebuffer_sRGB");
    hardcodedExtensions[KHR_texture_compression_astc_hdr]       = supports("GL_KHR_texture_compression_astc_hdr");
    hardcodedExtensions[KHR_texture_compression_astc_ldr]       = supports("GL_KHR_texture_compression_astc_ldr");
    hardcodedExtensions[KHR_texture_compression_astc_sliced_3d] = supports("GL_KHR_texture_compression_astc_sliced_3d");
    hardcodedExtensions[KHR_shader_subgroup]                    = supports("GL_KHR_shader_subgroup");
    isInitialized                                               = true;
}

auto GlExtensions::NumExtensions() -> int32_t {
    assert(isInitialized);
    return allExtensions.size();
}

auto GlExtensions::Supports(char const* extensionName) -> bool {
    assert(isInitialized);
    return allExtensions.find(extensionName) != allExtensions.end();
}

auto GlExtensions::Supports(GlExtensions::Name extensionName) -> bool {
    assert(isInitialized);
    return hardcodedExtensions[extensionName];
}

} // namespace engine::gl
