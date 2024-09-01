#include "engine/gl/Extensions.hpp"
#include "engine/gl/Capabilities.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

bool GlExtensions::isInitialized = false;
std::unordered_set<std::string, StringHash, std::equal_to<>> GlExtensions::allExtensions{};
bool GlExtensions::hardcodedExtensions[static_cast<size_t>(Name::NUM_HARDCODED_EXTENSIONS)]{};

void GlExtensions::Initialize() {
    if (isInitialized) { return; }
    GLint numExtensions;
    GLCALL(glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions));
    for (GLint i = 0; i < numExtensions; ++i) {
        GLubyte const* ext = glGetStringi(GL_EXTENSIONS, i);
        // NOTE: reinterpret_cast to char const* would compile and work, but it is UB
        size_t extSize     = std::char_traits<GLubyte>::length(ext);
        auto extensionName = std::string{ext, ext + extSize};
        // XLOG("Extension {}", extensionName);
        allExtensions.insert(extensionName);
    }
    // NOTE: std::string_view doesn't work even with transparent hashing (StringHash)
    auto supports = [](const char* ext) {
        bool supported = allExtensions.find(ext) != allExtensions.end();
        XLOG("Extension {} supported={}", ext, static_cast<int>(supported));
        return supported;
    };
    hardcodedExtensions[KHR_debug]                              = supports("GL_KHR_debug");
    hardcodedExtensions[KHR_no_error]                           = supports("GL_KHR_no_error");
    hardcodedExtensions[KHR_shader_subgroup]                    = supports("GL_KHR_shader_subgroup");
    hardcodedExtensions[KHR_texture_compression_astc_hdr]       = supports("GL_KHR_texture_compression_astc_hdr");
    hardcodedExtensions[KHR_texture_compression_astc_ldr]       = supports("GL_KHR_texture_compression_astc_ldr");
    hardcodedExtensions[KHR_texture_compression_astc_sliced_3d] = supports("GL_KHR_texture_compression_astc_sliced_3d");
    hardcodedExtensions[ARB_debug_output]                       = supports("GL_ARB_debug_output");
    hardcodedExtensions[ARB_ES3_2_compatibility]                = supports("GL_ARB_ES3_2_compatibility");
    hardcodedExtensions[ARB_invalidate_subdata]                 = supports("GL_ARB_invalidate_subdata");
    hardcodedExtensions[ARB_framebuffer_sRGB]                   = supports("GL_ARB_framebuffer_sRGB");
    hardcodedExtensions[ARB_texture_storage]                    = supports("GL_ARB_texture_storage");
    hardcodedExtensions[ARB_texture_storage_multisample]        = supports("GL_ARB_texture_storage_multisample");
    hardcodedExtensions[EXT_debug_label]                        = supports("GL_EXT_debug_label");
    hardcodedExtensions[EXT_debug_marker]                       = supports("GL_EXT_debug_marker");
    hardcodedExtensions[EXT_texture_filter_anisotropic]         = supports("GL_EXT_texture_filter_anisotropic");
    isInitialized                                               = true;
}

auto GlExtensions::NumExtensions() -> int32_t {
    assert(isInitialized);
    return allExtensions.size();
}

auto GlExtensions::Supports(const char* extensionName) -> bool {
    assert(isInitialized);
    return allExtensions.find(extensionName) != allExtensions.end();
}

auto GlExtensions::Supports(GlExtensions::Name extensionName) -> bool {
    assert(isInitialized);
    return hardcodedExtensions[extensionName];
}

} // namespace engine::gl
