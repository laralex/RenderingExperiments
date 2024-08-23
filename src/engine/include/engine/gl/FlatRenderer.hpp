#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

class FlatRenderer final {

public:
#define Self FlatRenderer
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] () -> FlatRenderer;
    void Render(
        Vao const& vaoWithNormal, GLenum primitive, glm::mat4 const& model, glm::mat4 const& camera,
        glm::vec3 lightPosition) const;

private:
    GpuProgram program_;
    GpuBuffer ubo_;
};

} // namespace engine::gl