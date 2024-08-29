#pragma once

#include "engine/Prelude.hpp"
#include <glm/vec4.hpp>

namespace engine::gl {

class GpuSampler final {

public:
#define Self GpuSampler
    explicit Self() noexcept = default;
    ~Self() noexcept { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (std::string_view name = {}) -> GpuSampler;
    auto WithDepthCompare [[nodiscard]] (bool enable, GLenum compareFunc = GL_LEQUAL) && -> GpuSampler&&;
    auto WithBorderColor [[nodiscard]] (glm::vec4 color) && -> GpuSampler&&;
    auto WithLinearMagnify [[nodiscard]] (bool filterLinear) && -> GpuSampler&&;
    auto WithLinearMinify [[nodiscard]] (bool filterLinear) && -> GpuSampler&&;
    auto WithLinearMinifyOverMips [[nodiscard]] (bool filterUsingMips, bool filterLinear) && -> GpuSampler&&;
    auto WithAnisotropicFilter [[nodiscard]] (GLfloat maxAnisotropy) && -> GpuSampler&&;
    auto WithMipConfig
        [[nodiscard]] (GLfloat minMip = -1000.0f, GLfloat maxMip = 1000.0f, GLfloat bias = 0.0f) && -> GpuSampler&&;
    auto WithWrap [[nodiscard]] (GLenum wrapX, GLenum wrapY, GLenum wrapZ) && -> GpuSampler&&;
    auto WithWrap [[nodiscard]] (GLenum wrapXYZ) && -> GpuSampler&&;
    auto Id [[nodiscard]] () const -> GLuint { return samplerId_; }

private:
    void Dispose();
    auto SetMinificationFilter [[nodiscard]] (bool minifyLinear, bool useMips, bool mipsLinear) && -> GpuSampler&&;

    GlHandle samplerId_{GL_NONE};
    GLenum magnificationFilter_{GL_NEAREST};
    GLenum minificationFilter_{GL_NEAREST};
    bool filterUsingMips_{false};
    bool filterLinearMips_{false};
    bool filterLinearMinify_{false};
};

class SamplerTiling final {

public:
#define Self SamplerTiling
    explicit Self(glm::vec2 uvScale = glm::vec2{1.0f}, glm::vec2 uvOffset = glm::vec2{0.0f}) noexcept
        : uvScale_(uvScale, 0.0f)
        , uvOffset_(uvOffset, 0.0f) {
        uvOffset_ = glm::fract(uvOffset_);
    }
    ~Self() noexcept             = default;
    Self(Self const&)            = default;
    Self& operator=(Self const&) = default;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    auto UvScale [[nodiscard]] () const -> glm::vec3 { return uvScale_; }
    auto UvOffset [[nodiscard]] () const -> glm::vec3 { return uvOffset_; }
    auto Packed [[nodiscard]] () const -> glm::vec4 {
        return glm::vec4{uvScale_.x, uvScale_.y, uvOffset_.x, uvOffset_.y};
    }

private:
    glm::vec3 uvScale_{0.0f};
    glm::vec3 uvOffset_{0.0f};
};

} // namespace engine::gl