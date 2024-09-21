#pragma once

#include "engine/Precompiled.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

class AxesRenderer final {

public:
#define Self AxesRenderer
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (GlContext const& gl) -> AxesRenderer;
    void Render(glm::mat4 const& mvp, float scale = 1.0f) const;

private:
    Vao vao_;
    GpuBuffer attributeBuffer_;
    GpuBuffer indexBuffer_;
    GpuProgram customizedProgram_;
    GpuProgram defaultProgram_;
};

} // namespace engine::gl
