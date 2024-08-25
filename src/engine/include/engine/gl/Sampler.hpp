#pragma once

#include "engine/Prelude.hpp"
#include <glm/vec4.hpp>

namespace engine::gl {

class Sampler final {

public:
#define Self Sampler
    explicit Self() noexcept = default;
    ~Self() noexcept { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate [[nodiscard]] (std::string_view name = {}) -> Sampler;
    auto WithDepthCompare [[nodiscard]] (bool enable, GLenum compareFunc = GL_LEQUAL) && -> Sampler&&;
    auto WithBorderColor [[nodiscard]] (glm::vec4 color) && -> Sampler&&;
    auto WithLinearMagnify [[nodiscard]] (bool filterLinear) && -> Sampler&&;
    auto WithLinearMinify [[nodiscard]] (bool filterLinear) && -> Sampler&&;
    auto WithLinearMinifyOverMips [[nodiscard]] (bool filterUsingMips, bool filterLinear) && -> Sampler&&;
    auto WithAnisotropicFilter [[nodiscard]] (GLfloat maxAnisotropy) && -> Sampler&&;
    auto WithMipConfig
        [[nodiscard]] (GLfloat minMip = -1000.0f, GLfloat maxMip = 1000.0f, GLfloat bias = 0.0f) && -> Sampler&&;
    auto WithWrap [[nodiscard]] (GLenum wrapX, GLenum wrapY, GLenum wrapZ) && -> Sampler&&;
    auto WithWrap [[nodiscard]] (GLenum wrapXYZ) && -> Sampler&&;
    auto Id [[nodiscard]] () const -> GLuint { return samplerId_; }

private:
    void Dispose();
    auto SetMinificationFilter [[nodiscard]] (bool minifyLinear, bool useMips, bool mipsLinear) && -> Sampler&&;

    GlHandle samplerId_{GL_NONE};
    GLenum magnificationFilter_{GL_NEAREST};
    GLenum minificationFilter_{GL_NEAREST};
    bool filterUsingMips_{false};
    bool filterLinearMips_{false};
    bool filterLinearMinify_{false};
};

} // namespace engine::gl