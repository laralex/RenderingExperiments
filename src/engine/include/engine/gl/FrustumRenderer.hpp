#pragma once

#include "engine/Precompiled.hpp"
#include "engine/Unprojection.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/IGlDisposable.hpp"
#include "engine/gl/GpuProgram.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>
namespace engine::gl {

class FrustumRenderer final : public IGlDisposable {

public:
#define Self FrustumRenderer
    explicit Self() noexcept     = default;
    ~Self() override             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (GlContext& gl) -> FrustumRenderer;
    void Render(
        GlContext const& gl, glm::mat4 const& originMvp, Frustum const& frustum, glm::vec4 color = glm::vec4(1.0),
        float thickness = 0.015f) const;
    void Dispose(GlContext const& gl) override;

private:
    Vao vao_ = Vao{};
    GpuBuffer attributeBuffer_ = GpuBuffer{};
    GpuBuffer indexBuffer_ = GpuBuffer{};
    std::shared_ptr<GpuProgram> program_ = {};
    GpuBuffer ubo_ = GpuBuffer{};
    GLint uboLocation_ = -1;
};

} // namespace engine::gl
