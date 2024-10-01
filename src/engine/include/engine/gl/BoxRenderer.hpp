#pragma once

#include "engine/Precompiled.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/GpuProgram.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

class BoxRenderer final : public IGlDisposable {

public:
#define Self BoxRenderer
    explicit Self() noexcept     = default;
    ~Self() override             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (GlContext& gl) -> BoxRenderer;
    void Render(GlContext const& gl, glm::mat4 const& centerMvp, glm::vec4 color) const;
    void Dispose(GlContext const& gl) override;

private:
    Vao vao_;
    GpuBuffer attributeBuffer_;
    GpuBuffer indexBuffer_;
    std::shared_ptr<GpuProgram> program_ = {};
};

} // namespace engine::gl
