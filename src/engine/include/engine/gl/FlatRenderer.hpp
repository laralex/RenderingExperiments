#pragma once

#include "engine/Precompiled.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Vao.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

struct FlatRenderArgs;

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
    void Render(FlatRenderArgs const&) const;

private:
    GpuProgram program_{};
    GpuBuffer ubo_{};
    GLint uboLocation_{-1};
};

struct FlatRenderArgs {
    glm::vec4 lightWorldPosition;
    glm::vec3 lightColor;
    glm::vec3 materialColor;
    GLenum primitive;
    Vao const& vaoWithNormal;
    glm::mat4 const& mvp;
    glm::mat4 const& modelToWorld;
};

} // namespace engine::gl