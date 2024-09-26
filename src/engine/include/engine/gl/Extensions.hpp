#pragma once

#include <glad/gl.h>
#include <cassert>
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
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
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

    void Initialize();
    auto IsInitialized [[nodiscard]] () const -> bool { return isInitialized_; }
    auto NumExtensions [[nodiscard]] () const -> int32_t {
        assert(isInitialized_);
        return allExtensions_.size();
    }
    // NOTE: std::string_view doesn't work even with transparent hashing (StringHash)
    auto Supports [[nodiscard]] (char const* extensionName) const -> bool {
        assert(isInitialized_);
        return allExtensions_.find(extensionName) != allExtensions_.end();
    }
    auto Supports [[nodiscard]] (GlExtensions::Name extensionName) const -> bool {
        assert(isInitialized_);
        return hardcodedExtensions_[extensionName];
    }

private:
    bool isInitialized_{false};
    std::unordered_set<std::string> allExtensions_{};
    bool hardcodedExtensions_[NUM_HARDCODED_EXTENSIONS]{};
};

} // namespace engine::gl