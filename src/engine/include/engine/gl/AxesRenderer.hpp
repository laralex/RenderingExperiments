#pragma once

#include "engine/FirstPersonLocomotion.hpp"
#include "engine/Precompiled.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/GpuProgram.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

class AxesRenderer final : public IGlDisposable {

public:
#define Self AxesRenderer
    explicit Self() noexcept     = default;
    ~Self() override             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (GlContext const& gl) -> AxesRenderer;
    void Render(GlContext const& gl, glm::mat4 const& mvp, float scale = 1.0f) const;
    void Dispose(GlContext const& gl) override;

private:
    Vao vao_                            = Vao{};
    GpuBuffer attributeBuffer_          = GpuBuffer{};
    GpuBuffer indexBuffer_              = GpuBuffer{};
    GpuProgramHandle customizedProgram_ = GpuProgramHandle{};
    GpuProgramHandle defaultProgram_    = GpuProgramHandle{};
};

} // namespace engine::gl
