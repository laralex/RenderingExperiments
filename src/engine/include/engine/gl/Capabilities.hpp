#pragma once

#include <glad/gl.h>
#include <string_view>
#include <string>

namespace engine::gl {

class GlCapabilities final {

public:
#define Self GlCapabilities
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    void Initialize();
    auto IsInitialized [[nodiscard]] () const -> bool { return isInitialized_; };
    auto Vendor [[nodiscard]] () const -> std::string_view { return vendor_; }
    auto VendorDevice [[nodiscard]] () const -> std::string_view { return renderer_; }

    GLint maxTextureUnits;
    GLint numExtensions;
    GLint majorVersion;
    GLint minorVersion;
    GLint maxUboBindings;
    GLint maxUboBlocksVertex;
    GLint maxUboBlocksFragment;
    GLint maxUboBlocksGeometry;
    GLint uboOffsetAlignment;
    GLint maxDrawBuffers;

private:
    bool isInitialized_;
    std::string vendor_{"DEVICE_VENDOR_N/A"};
    std::string renderer_{"DEVICE_N/A"};
};

} // namespace engine::gl
