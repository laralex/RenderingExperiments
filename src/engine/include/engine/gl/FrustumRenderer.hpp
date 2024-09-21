#pragma once

#include "engine/Precompiled.hpp"
#include "engine/Unprojection.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>
namespace engine::gl {

class FrustumRenderer final {

public:
#define Self FrustumRenderer
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (GlContext const& gl) -> FrustumRenderer;
    void Render(
        glm::mat4 const& originMvp, Frustum const& frustum, glm::vec4 color = glm::vec4(1.0),
        float thickness = 0.015f) const;

private:
    Vao vao_{};
    GpuBuffer attributeBuffer_{};
    GpuBuffer indexBuffer_{};
    GpuProgram program_{};
    GpuBuffer ubo_{};
    GLint uboLocation_{-1};
};

} // namespace engine::gl
