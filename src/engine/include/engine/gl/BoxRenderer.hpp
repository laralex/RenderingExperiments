#pragma once

#include "engine/Precompiled.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

class BoxRenderer final {

public:
#define Self BoxRenderer
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] () -> BoxRenderer;
    void Render(glm::mat4 const& centerMvp, glm::vec4 color) const;

private:
    Vao vao_;
    GpuBuffer attributeBuffer_;
    GpuBuffer indexBuffer_;
    GpuProgram program_;
};

} // namespace engine::gl
