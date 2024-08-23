#pragma once

#include "engine/Prelude.hpp"
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

    static auto Allocate [[nodiscard]] () -> AxesRenderer;
    void Render(glm::mat4 const& mvp, glm::vec3 scale = glm::vec3{1.0f}) const;

private:
    Vao vao;
    GpuBuffer attributeBuffer;
    GpuBuffer indexBuffer;
    GpuProgram customizedProgram;
    GpuProgram defaultProgram;
};

} // namespace engine::gl
