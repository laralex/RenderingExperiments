#pragma once

#include "engine/Prelude.hpp"

namespace engine::gl {

class GlCapabilities final {

public:
#define Self GlCapabilities
    explicit Self() noexcept     = delete;
    ~Self() noexcept             = delete;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    static void Initialize();
    static auto IsInitialized [[nodiscard]] () -> bool { return isInitialized; };
    static GLint maxTextureUnits;
    static GLint numExtensions;
    static GLint majorVersion;
    static GLint minorVersion;
    static GLint maxUboBindings;
    static GLint maxUboBlocksVertex;
    static GLint maxUboBlocksFragment;
    static GLint maxUboBlocksGeometry;
    static GLint uboOffsetAlignment;

private:
    static bool isInitialized;
};

} // namespace engine::gl
