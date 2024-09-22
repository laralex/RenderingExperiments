#pragma once

#include "engine/Precompiled.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace engine::gl {

class Renderbuffer final {

public:
#define Self Renderbuffer
    explicit Self() noexcept = default;
    ~Self() noexcept { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate2D [[nodiscard]] (
        GlContext const& gl, glm::ivec2 size, GLenum internalFormat, int32_t msaaSamples = 0,
        std::string_view name = {}) -> Renderbuffer;

    auto Id [[nodiscard]] () const -> GLuint { return renderbufferId_; }
    auto Size [[nodiscard]] () const -> glm::ivec3 { return size_; }
    auto RenderbufferSlotTarget [[nodiscard]] () const -> GLenum { return target_; }
    auto MsaaSamples [[nodiscard]] () const -> std::optional<int32_t>;
    auto IsMultisampled [[nodiscard]] () const -> bool { return msaaSamples_ > 0; };

private:
    void Dispose();

    GlHandle renderbufferId_{GL_NONE};
    GLenum target_{GL_NONE};
    GLenum internalFormat_{GL_NONE};
    GLsizei msaaSamples_{0};
    glm::ivec3 size_{};

    friend class RenderbufferCtx;
};

// Helper object, binds GL renderbuffer in ctor, unbinds it in dtor
// No two instances of the class should exist in one scope (checked by assert)
class RenderbufferCtx final {
public:
#define Self RenderbufferCtx
    explicit Self(Renderbuffer const& useRenderbuffer) noexcept;
    ~Self() noexcept;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

private:
    static GlHandle contextRenderbuffer_;
    static GLenum contextTarget_;
    static bool hasInstances_;
};

} // namespace engine::gl