#pragma once

#include "engine/Prelude.hpp"

namespace engine::gl {

class Texture;
class Renderbuffer;
class FramebufferCtx;

class Framebuffer final {

public:
#define Self Framebuffer
    explicit Self() noexcept = default;
    ~Self() noexcept { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static void BindBackbuffer();
    static auto Allocate [[nodiscard]] (std::string_view name = {}) -> Framebuffer;

    auto Id [[nodiscard]] () const -> GLuint { return fbId_; }
    void BindDraw() const;
    void BindRead() const;
    auto IsComplete [[nodiscard]] () const -> bool;

private:
    void Dispose();
    GlHandle fbId_{GL_NONE};
    int32_t numAttachments_{0};

    friend class FramebufferCtx;
};

// Helper object, binds GL FBO in ctor, unbinds it in dtor
// No two instances of the class should exist in one scope (checked by assert)
class FramebufferCtx final {
public:
#define Self FramebufferCtx
    explicit Self(Framebuffer const& useFramebuffer, bool draw = false) noexcept;
    explicit Self(GLuint useFramebuffer, bool draw = false) noexcept;
    ~Self() noexcept;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    auto ClearColor (GLint drawBufferIdx, GLint r, GLint g, GLint b, GLint a) const -> FramebufferCtx const&;
    auto ClearColor (GLint drawBufferIdx, GLuint r, GLuint g, GLuint b, GLuint a) const -> FramebufferCtx const&;
    auto ClearColor (GLint drawBufferIdx, GLfloat r, GLfloat g, GLfloat b, GLfloat a) const -> FramebufferCtx const&;
    auto ClearDepth (GLfloat value) const -> FramebufferCtx const&;
    auto ClearStencil (GLint value) const -> FramebufferCtx const&;
    auto ClearDepthStencil (GLfloat depth, GLint stencil) const -> FramebufferCtx const&;
    auto Invalidate (uint32_t numAttachments, GLenum* attachments) const -> FramebufferCtx const&;

    auto LinkTexture (GLenum attachment, Texture const& tex, GLint texLevel = 0, GLint arrayIndex = -1) const
    -> FramebufferCtx const&;
    auto LinkRenderbuffer (GLenum attachment, Renderbuffer const& rb, GLint arrayIndex = -1) const
    -> FramebufferCtx const&;
    // auto LinkBackbuffer(GLenum attachment, GLint texLevel = 0) const -> FramebufferCtx&&;

    auto IsComplete [[nodiscard]] () const -> bool;

private:
    static GlHandle contextFramebuffer_;
    static GLenum framebufferTarget_;
    static bool hasInstances_;
};

} // namespace engine::gl
