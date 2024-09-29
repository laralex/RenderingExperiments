#pragma once

#include "engine/Log.hpp"
#include "engine/gl/Capabilities.hpp"
#include "engine/gl/Extensions.hpp"
#include "engine/gl/TextureUnits.hpp"
#include "engine/gl/GpuProgramHandle.hpp"
#include <memory>

namespace engine::gl {

class GpuProgramOwner;
class GpuProgram;

class GlContext final {

public:
#define Self GlContext
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    void Initialize();
    auto IsInitialized [[nodiscard]] () const -> bool { return isInitialized_; }
    auto Extensions [[nodiscard]] () const -> GlExtensions const& { return extensions_; }
    auto Capabilities [[nodiscard]] () const -> GlCapabilities const& { return capabilities_; }
    auto TextureUnits [[nodiscard]] () -> GlTextureUnits& { return textureUnits_; }
    auto GetProgram [[nodiscard]] (GpuProgramHandle const& handle) const -> GpuProgram const&;
    auto Programs [[nodiscard]] () const -> std::shared_ptr<GpuProgramOwner> { return programOwner_; }

private:
    bool isInitialized_{false};
    GlExtensions extensions_{};
    GlCapabilities capabilities_{};
    GlTextureUnits textureUnits_{};
    std::shared_ptr<GpuProgramOwner> programOwner_{};
};

} // namespace engine::gl