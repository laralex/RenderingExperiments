#pragma once

#include "engine/Precompiled.hpp"
#include "engine/gl/GpuBuffer.hpp"
#include "engine/gl/Context.hpp"
#include "engine/gl/IGlDisposable.hpp"
#include "engine/gl/GpuProgram.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

class EditorGridRenderer final : public IGlDisposable {
#define Self EditorGridRenderer

public:
    explicit Self() noexcept     = default;
    ~Self() override             = default;
    // TODO: fix slicing for all polymorphic types
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;

    struct RenderArgs {
        glm::vec4 thickColor = glm::vec4{0.0f};
        glm::vec4 thinColor = glm::vec4{0.1f};
        glm::vec3 cameraWorldPosition;
        glm::mat4 const& viewProjection;
    };

    static auto Allocate [[nodiscard]] (GlContext& gl) -> EditorGridRenderer;
    void Render(GlContext& gl, RenderArgs const&) const;
    void Dispose(GlContext const& gl) override;

private:
    std::shared_ptr<GpuProgram> program_ = {};
    GpuBuffer ubo_ = GpuBuffer{};
    GLint uboLocation_ = -1;
#undef Self
};



} // namespace engine::gl