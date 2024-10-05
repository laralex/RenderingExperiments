#pragma once

#include "engine/Log.hpp"
#include "engine/gl/Common.hpp"
#include "engine/gl/Capabilities.hpp"
#include "engine/gl/Vao.hpp"
#include "engine/gl/Extensions.hpp"
#include "engine/gl/TextureUnits.hpp"
#include "engine/gl/GpuProgramRegistry.hpp"
#include <memory>

namespace engine::gl {

class GpuProgramRegistry;
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
    auto Programs [[nodiscard]] () const -> std::shared_ptr<GpuProgramRegistry> { return programsRegistry_; }

    auto VaoDatalessTriangle [[nodiscard]] () const -> Vao const& { return datalessTriangleVao_; }
    auto VaoDatalessQuad [[nodiscard]] () const -> Vao const& { return datalessQuadVao_; }

private:
    bool isInitialized_{false};
    GlExtensions extensions_{};
    GlCapabilities capabilities_{};
    GlTextureUnits textureUnits_{};
    // NOTE: it's a shared ptr, because it's given by a weak ptr into filesystem watcher
    std::shared_ptr<GpuProgramRegistry> programsRegistry_{};

    Vao datalessTriangleVao_ = Vao{};
    Vao datalessQuadVao_ = Vao{};
};

} // namespace engine::gl