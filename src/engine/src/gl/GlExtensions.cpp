#include "engine/gl/GlExtensions.hpp"
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
    auto supports = [&](char const* ext, bool procAddressOk) {
        bool supported = allExtensions_.find(ext) != allExtensions_.end();
        XLOGD("Extension {} supported={} procAddressOk={}", ext, static_cast<int>(supported), procAddressOk);
        return supported && procAddressOk;
    };
    constexpr bool OK                                      = true;
    hardcodedExtensions_[KHR_debug]                        = supports("GL_KHR_debug", glGetObjectLabel != nullptr);
    hardcodedExtensions_[KHR_no_error]                     = supports("GL_KHR_no_error", OK);
    hardcodedExtensions_[KHR_shader_subgroup]              = supports("GL_KHR_shader_subgroup", OK);
    hardcodedExtensions_[KHR_texture_compression_astc_hdr] = supports("GL_KHR_texture_compression_astc_hdr", OK);
    hardcodedExtensions_[KHR_texture_compression_astc_ldr] = supports("GL_KHR_texture_compression_astc_ldr", OK);
    hardcodedExtensions_[KHR_texture_compression_astc_sliced_3d] =
        supports("GL_KHR_texture_compression_astc_sliced_3d", OK);
    hardcodedExtensions_[ARB_buffer_storage]      = supports("GL_ARB_buffer_storage", glBufferStorage != nullptr);
    hardcodedExtensions_[ARB_debug_output]        = supports("GL_ARB_debug_output", glGetObjectLabel != nullptr);
    hardcodedExtensions_[ARB_ES3_2_compatibility] = supports("GL_ARB_ES3_2_compatibility", OK);
    hardcodedExtensions_[ARB_invalidate_subdata] =
        supports("GL_ARB_invalidate_subdata", glInvalidateFramebuffer != nullptr);
    hardcodedExtensions_[ARB_framebuffer_sRGB] = supports("GL_ARB_framebuffer_sRGB", OK);
    hardcodedExtensions_[ARB_shading_language_include] =
        supports("GL_ARB_shading_language_include", glNamedStringARB != nullptr);
    hardcodedExtensions_[ARB_texture_filter_anisotropic] = supports("GL_ARB_texture_filter_anisotropic", OK);
    hardcodedExtensions_[ARB_texture_storage] = supports("GL_ARB_texture_storage", glTexStorage2D != nullptr);
    hardcodedExtensions_[ARB_texture_storage_multisample] =
        supports("GL_ARB_texture_storage_multisample", glTexStorage2DMultisample != nullptr);
    hardcodedExtensions_[EXT_debug_label]  = supports("GL_EXT_debug_label", glLabelObjectEXT != nullptr);
    hardcodedExtensions_[EXT_debug_marker] = supports("GL_EXT_debug_marker", glPushGroupMarkerEXT != nullptr);
    isInitialized_                         = true;
}

} // namespace engine::gl
