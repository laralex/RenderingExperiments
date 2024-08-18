#pragma once

#include "engine/Prelude.hpp"

namespace engine::gl {

class Texture;
class Renderbuffer;

class Framebuffer final {

public:
#define Self Framebuffer
    explicit Self() = default;
    ~Self() { Dispose(); };
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
    auto IsComplete [[nodiscard]] () -> bool;

private:
    void Dispose();
    GlHandle fbId_{GL_NONE};
    int32_t numAttachments_{0};
};

// Helper object, binds GL FBO in ctor, unbinds it in dtor
// No two instances of the class should exist in one scope (checked by assert)
class FramebufferCtx final {
public:
#define Self FramebufferCtx
    explicit Self(Framebuffer const& useFramebuffer, bool draw = false);
    explicit Self(GLuint useFramebuffer, bool draw = false);
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    auto ClearColor [[nodiscard]] (GLint drawBufferIdx, GLint r, GLint g, GLint b, GLint a) -> FramebufferCtx&&;
    auto ClearColor [[nodiscard]] (GLint drawBufferIdx, GLuint r, GLuint g, GLuint b, GLuint a) -> FramebufferCtx&&;
    auto ClearColor [[nodiscard]] (GLint drawBufferIdx, GLfloat r, GLfloat g, GLfloat b, GLfloat a) -> FramebufferCtx&&;
    auto ClearDepth [[nodiscard]] (GLfloat value) -> FramebufferCtx&&;
    auto ClearStencil [[nodiscard]] (GLint value) -> FramebufferCtx&&;
    auto ClearDepthStencil [[nodiscard]] (GLfloat depth, GLint stencil) -> FramebufferCtx&&;
    auto Invalidate [[nodiscard]] (uint32_t numAttachments, GLenum* attachments) -> FramebufferCtx&&;

    auto LinkTexture [[nodiscard]] (GLenum attachment, Texture const& tex, GLint texLevel = 0, GLint arrayIndex = -1)
    -> FramebufferCtx&&;
    auto LinkRenderbuffer [[nodiscard]] (GLenum attachment, Renderbuffer const& rb, GLint arrayIndex = -1)
    -> FramebufferCtx&&;
    // auto LinkBackbuffer(GLenum attachment, GLint texLevel = 0) const -> FramebufferCtx&&;

    auto IsComplete [[nodiscard]] () -> bool;
private:
    static GlHandle contextFramebuffer_;
    static GLenum framebufferTarget_;
    static bool hasInstances_;
};

} // namespace engine::gl
