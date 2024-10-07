#pragma once

#include "engine/Precompiled.hpp"

namespace engine::gl {

class Texture;
class Renderbuffer;
class FramebufferDrawCtx;
class FramebufferEditCtx;

class Framebuffer final {

public:
#define Self Framebuffer
    explicit Self() noexcept;
    ~Self() noexcept { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static void BindBackbuffer();
    static auto Allocate [[nodiscard]] (GlContext& gl, std::string_view name = {}) -> Framebuffer;

    auto Id [[nodiscard]] () const -> GLuint { return fbId_; }
    void BindDraw() const;
    void BindRead() const;

private:
    void Dispose();

    // TODO: assert with capabilities
    constexpr static size_t MAX_DRAW_BUFFERS = 8U;
    GlHandle fbId_{GL_NONE};
    GLenum drawBuffers_[MAX_DRAW_BUFFERS] = {GL_COLOR_ATTACHMENT0}; // others are GL_NONE

    friend class FramebufferEditCtx;
};

// Helper object, binds GL FBO in ctor, unbinds it in dtor
// No two instances of the class should exist in one scope (checked by assert)
class FramebufferDrawCtx final {
public:
#define Self FramebufferDrawCtx
    explicit Self(Framebuffer const& useFramebuffer, bool bindAsDraw = true) noexcept;
    explicit Self(GLuint useFramebuffer, bool bindAsDraw = true) noexcept;
    ~Self() noexcept;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self
    void GuardAnother(GLuint useFramebuffer, bool bindAsDraw = true) noexcept;

    auto ClearColor(GLint drawBufferIdx, GLint r, GLint g, GLint b, GLint a) const -> FramebufferDrawCtx const&;
    auto ClearColor(GLint drawBufferIdx, GLuint r, GLuint g, GLuint b, GLuint a) const -> FramebufferDrawCtx const&;
    auto ClearColor(GLint drawBufferIdx, GLfloat r, GLfloat g, GLfloat b, GLfloat a) const -> FramebufferDrawCtx const&;
    auto ClearDepth(GLfloat value) const -> FramebufferDrawCtx const&;
    auto ClearStencil(GLint value) const -> FramebufferDrawCtx const&;
    auto ClearDepthStencil(GLfloat depth, GLint stencil) const -> FramebufferDrawCtx const&;
    auto Invalidate(GlContext const& gl, CpuMemory<GLenum const> attachments) const -> FramebufferDrawCtx const&;

    auto IsComplete [[nodiscard]] (GlContext const& gl) const -> bool;

    auto BoundTarget() const -> GLenum { return framebufferTarget_; }
    static auto IsContextExisting() -> bool { return hasInstances_; }

private:
    ENGINE_STATIC static GlHandle contextFramebuffer_;
    ENGINE_STATIC static GLenum framebufferTarget_;
    ENGINE_STATIC static bool hasInstances_;
};

// Helper object, binds GL FBO in ctor, unbinds it in dtor
// No two instances of the class should exist in one scope (checked by assert)
class FramebufferEditCtx final {
public:
#define Self FramebufferEditCtx
    explicit Self(Framebuffer& useFramebuffer, bool bindAsDraw = true) noexcept;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = delete;
#undef Self
    auto AttachTexture(
        GlContext& gl, GLenum attachment, Texture const& tex, GLint texLevel = 0, GLint arrayIndex = -1) const
        -> FramebufferEditCtx const&;
    auto AttachRenderbuffer(GlContext& gl, GLenum attachment, Renderbuffer const& rb, GLint arrayIndex = -1) const
        -> FramebufferEditCtx const&;
    auto CommitDrawbuffers() const -> FramebufferEditCtx const&;
    auto SetReadbuffer(GLenum attachment) const -> FramebufferEditCtx const&;
    // auto SetDrawBuffers(CpuView<GLenum> attachments) const -> FramebufferEditCtx const&;
    // auto SetDrawBuffers(CpuView<GLenum> attachments) const -> FramebufferEditCtx const&;

    auto IsComplete [[nodiscard]] (GlContext& gl) const -> bool { return ctx_.IsComplete(gl); };

private:
    Framebuffer& fb_;
    FramebufferDrawCtx ctx_;
};

} // namespace engine::gl
