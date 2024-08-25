#pragma once

#include "engine/Prelude.hpp"
#include "engine/BoxMesh.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Vao.hpp"

namespace engine::gl {

class GpuMesh final {

public:
    struct AttributesLayout;

#define Self GpuMesh
    explicit Self(Vao&& vao, GpuBuffer&& positions, GpuBuffer&& attributes, GpuBuffer&& indices, AttributesLayout layout, GLenum trignagleWinding = GL_CCW) noexcept
        : vao_(std::move(vao)), positionBuffer_(std::move(positions)), attributesBuffer_(std::move(attributes)), indexBuffer_(std::move(indices)), attributesLayout_(layout) {}
    explicit Self() noexcept = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    struct AttributesLayout final {
        GLuint positionLocation = 0;
        GLuint uvLocation = 1;
        GLuint normalLocation = 2;
    };

    auto Vao[[nodiscard]]() const -> Vao const& { return vao_; }
    auto FrontFace[[nodiscard]]() const -> GLenum { return frontFace_; }

private:
    engine::gl::Vao vao_{};
    GpuBuffer attributesBuffer_{};
    GpuBuffer positionBuffer_{};
    GpuBuffer indexBuffer_{};
    AttributesLayout attributesLayout_{};
    GLenum frontFace_{GL_CCW};
};

} // namespace engine::gl