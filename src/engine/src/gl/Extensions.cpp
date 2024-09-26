#include "engine/gl/Extensions.hpp"
#include "engine/Precompiled.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_EXPORT void GlExtensions::Initialize() {
    if (isInitialized_) { return; }
    GLint numExtensions;
    GLCALL(glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions));
    for (GLint i = 0; i < numExtensions; ++i) {
        GLubyte const* ext;
        GLCALL(ext = glGetStringi(GL_EXTENSIONS, i));
        // NOTE: reinterpret_cast to char const* would compile and work, but it is UB
        size_t extSize     = std::char_traits<GLubyte>::length(ext);
        auto extensionName = std::string{ext, ext + extSize};
        // XLOG("Extension {}", extensionName);
        allExtensions_.insert(extensionName);
    }
    // NOTE: std::string_view doesn't work even with transparent hashing (StringHash)
    auto supports = [&](const char* ext) {
        bool supported = allExtensions_.find(ext) != allExtensions_.end();
        XLOG("Extension {} supported={}", ext, static_cast<int>(supported));
        return supported;
    };
    hardcodedExtensions_[KHR_debug]                        = supports("GL_KHR_debug");
    hardcodedExtensions_[KHR_no_error]                     = supports("GL_KHR_no_error");
    hardcodedExtensions_[KHR_shader_subgroup]              = supports("GL_KHR_shader_subgroup");
    hardcodedExtensions_[KHR_texture_compression_astc_hdr] = supports("GL_KHR_texture_compression_astc_hdr");
    hardcodedExtensions_[KHR_texture_compression_astc_ldr] = supports("GL_KHR_texture_compression_astc_ldr");
    hardcodedExtensions_[KHR_texture_compression_astc_sliced_3d] =
        supports("GL_KHR_texture_compression_astc_sliced_3d");
    hardcodedExtensions_[ARB_debug_output]                = supports("GL_ARB_debug_output");
    hardcodedExtensions_[ARB_ES3_2_compatibility]         = supports("GL_ARB_ES3_2_compatibility");
    hardcodedExtensions_[ARB_invalidate_subdata]          = supports("GL_ARB_invalidate_subdata");
    hardcodedExtensions_[ARB_framebuffer_sRGB]            = supports("GL_ARB_framebuffer_sRGB");
    hardcodedExtensions_[ARB_shading_language_include]    = supports("GL_ARB_shading_language_include");
    hardcodedExtensions_[ARB_texture_storage]             = supports("GL_ARB_texture_storage");
    hardcodedExtensions_[ARB_texture_storage_multisample] = supports("GL_ARB_texture_storage_multisample");
    hardcodedExtensions_[EXT_debug_label]                 = supports("GL_EXT_debug_label");
    hardcodedExtensions_[EXT_debug_marker]                = supports("GL_EXT_debug_marker");
    hardcodedExtensions_[EXT_texture_filter_anisotropic]  = supports("GL_EXT_texture_filter_anisotropic");
    isInitialized_                                        = true;
}

} // namespace engine::gl
