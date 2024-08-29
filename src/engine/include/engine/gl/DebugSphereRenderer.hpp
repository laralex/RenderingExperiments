#pragma once

#include "engine/Prelude.hpp"
#include "engine/SphereRendererInput.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

#include <stack>

namespace engine::gl {

class DebugSphereRenderer final {

public:
#define Self DebugSphereRenderer
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (size_t maxSpheres) -> DebugSphereRenderer;
    void Fill(std::vector<SphereRendererInput::Sphere> const&, int32_t numSpheres, int32_t numSpheresOffset);
    void LimitInstances(int32_t numInstances);
    void Render(
        glm::mat4 const& camera, int32_t firstInstance = 0,
        int32_t numInstances = std::numeric_limits<int32_t>::max()) const;

private:
    Vao vao_{};
    GpuBuffer meshPositionsBuffer_{};
    GpuBuffer meshAttributesBuffer_{};
    GpuBuffer instancesBuffer_{};
    GpuBuffer indexBuffer_{};
    GpuProgram program_{};
    GLsizei lastInstance_{0};
};

} // namespace engine::gl