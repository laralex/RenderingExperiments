#pragma once

#include "engine/PointRendererInput.hpp"
#include "engine/Precompiled.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/GpuProgram.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

#include <stack>

namespace engine::gl {

class PointRenderer final : public IGlDisposable {

public:
#define Self PointRenderer
    explicit Self() noexcept     = default;
    ~Self() override             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (GlContext& gl, size_t maxPoints) -> PointRenderer;
    void Fill(std::vector<PointRendererInput::Point> const&, int32_t numPoints, int32_t numPointsOffset);
    void LimitInstances(int32_t numInstances);
    void Render(
        GlContext const& gl, glm::mat4 const& camera, int32_t firstInstance = 0,
        int32_t numInstances = std::numeric_limits<int32_t>::max()) const;
    void Dispose(GlContext const& gl) override;

private:
    Vao vao_ = Vao{};
    GpuBuffer meshPositionsBuffer_ = GpuBuffer{};
    GpuBuffer meshAttributesBuffer_ = GpuBuffer{};
    GpuBuffer instancesBuffer_ = GpuBuffer{};
    GpuBuffer indexBuffer_ = GpuBuffer{};
    std::shared_ptr<GpuProgram> program_ = {};
    GLsizei lastInstance_ = 0;
};

} // namespace engine::gl