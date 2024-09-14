#pragma once

#include "engine/Precompiled.hpp"
#include <glad/gl.h>

#include <unordered_set>

namespace engine {
struct StringHash;
//     struct StringEqual;
} // namespace engine

namespace engine::gl {

// NOTE: this class doesn't manage function pointer loading
// they're EXPECTED to be loaded by GLAD+GLFW

class GlExtensions final {

public:
#define Self GlExtensions
    explicit Self() noexcept     = delete;
    ~Self() noexcept             = delete;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    // A bunch of hardcoded extensions for faster runtime checking if they are supported
    enum Name {
        KHR_debug = 0,
        KHR_no_error,
        KHR_shader_subgroup,
        KHR_texture_compression_astc_hdr,
        KHR_texture_compression_astc_ldr,
        KHR_texture_compression_astc_sliced_3d,
        ARB_debug_output,
        ARB_ES3_2_compatibility,
        ARB_invalidate_subdata,
        ARB_framebuffer_sRGB,
        ARB_shading_language_include,
        ARB_texture_storage,
        ARB_texture_storage_multisample,
        EXT_debug_label,
        EXT_debug_marker,
        EXT_texture_filter_anisotropic,
        NUM_HARDCODED_EXTENSIONS
    };

    static void Initialize();
    static auto IsInitialized [[nodiscard]] () -> bool { return isInitialized; }
    static auto NumExtensions [[nodiscard]] () -> int32_t;
    // NOTE: std::string_view doesn't work even with transparent hashing (StringHash)
    static auto Supports [[nodiscard]] (char const* extensionName) -> bool;
    static auto Supports [[nodiscard]] (GlExtensions::Name extensionName) -> bool;

private:
    static bool isInitialized;
    static std::unordered_set<std::string, engine::StringHash, std::equal_to<>> allExtensions;
    static bool hardcodedExtensions[NUM_HARDCODED_EXTENSIONS];
};

} // namespace engine::gl