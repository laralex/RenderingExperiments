#pragma once

#include "engine/Prelude.hpp"
#include <unordered_set>

namespace engine::gl {

// NOTE: this class doesn't manage function pointer loading
// they're EXPECTED to be loaded by GLAD+GLFW

class GlExtensions final {

public:
#define Self GlExtensions
    explicit Self()              = delete;
    ~Self()                      = delete;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    enum Name {
        KHR_debug = 0,
        ARB_framebuffer_sRGB,
        KHR_texture_compression_astc_hdr,
        KHR_texture_compression_astc_ldr,
        KHR_texture_compression_astc_sliced_3d,
        KHR_shader_subgroup,
        NUM_HARDCODED_EXTENSIONS
    };

    static void Initialize();
    static auto IsInitialized() -> bool { return isInitialized; }
    static auto NumExtensions() -> int32_t;
    static auto Supports(char const* extensionName) -> bool;
    static auto Supports(GlExtensions::Name extensionName) -> bool;

private:
    static bool isInitialized;
    static std::unordered_set<std::string> allExtensions;
    static bool hardcodedExtensions[NUM_HARDCODED_EXTENSIONS];
};

} // namespace engine::gl