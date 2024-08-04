#pragma once

#include "engine/MemoryOp.hpp"
#include "engine/Prelude.hpp"

namespace engine::gl {

class GlGuardBindings final {

public:
#define Self GlGuardBindings
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self();
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLenum activeTexture           = 0xDEAD;
    GLuint vbo                     = 0xDEAD;
    GLuint program                 = 0xDEAD;
    GLuint dispatchIndirectBuffer  = 0xDEAD;
    GLuint drawIndirectBuffer      = 0xDEAD;
    GLuint drawFramebuffer         = 0xDEAD;
    GLuint readFramebuffer         = 0xDEAD;
    GLuint ebo                     = 0xDEAD;
    GLuint pixelPackBuffer         = 0xDEAD;
    GLuint pixelUnpackBuffer       = 0xDEAD;
    GLuint programPipeline         = 0xDEAD;
    GLuint renderBuffer            = 0xDEAD;
    GLuint primitiveRestartIndex   = 0xDEAD;
    GLuint vao                     = 0xDEAD;
    GLuint textureBuffer           = 0xDEAD;
    GLuint transformFeedbackBuffer = 0xDEAD;
};

class GlGuardFlags final {

public:
#define Self GlGuardFlags
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self();
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLboolean blend       = GL_FALSE;
    GLboolean depthTest   = GL_FALSE;
    GLboolean multisample = GL_FALSE;
    GLboolean stencilTest = GL_FALSE;
};

class GlGuardFlagsRare final {

public:
#define Self GlGuardFlagsRare
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self();
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLboolean colorLogicOp       = GL_FALSE;
    GLboolean cullFace           = GL_FALSE;
    GLboolean depthClamp         = GL_FALSE;
    GLboolean framebufferSrgb    = GL_FALSE;
    GLboolean polygonOffsetFill  = GL_FALSE;
    GLboolean polygonOffsetLine  = GL_FALSE;
    GLboolean polygonOffsetPoint = GL_FALSE;
    GLboolean primitiveRestart   = GL_FALSE;
    GLboolean scissorTest        = GL_FALSE;
};

class GlGuardDepth final {

public:
#define Self GlGuardDepth
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self();
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLboolean depthTest         = GL_FALSE;
    GLfloat depthClearValue     = 0.424242f;
    GLenum depthFunc            = 0xDEAD;
    GLfloat depthRange[2]       = {42.42f, 42.42f};
    GLboolean depthMask         = GL_FALSE;
    GLfloat polygonOffsetFactor = 0.f;
    GLfloat polygonOffsetUnits  = 0.f;
};

class GlGuardStencil final {

public:
#define Self GlGuardStencil
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self();
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLboolean stencilTest            = GL_FALSE;
    GLint stencilClearValue          = 0xDEAD;
    GLenum stencilBackFail           = 0xDEAD;
    GLenum stencilBackFunc           = 0xDEAD;
    GLenum stencilBackPassDepthFail  = 0xDEAD;
    GLenum stencilBackPassDepthPass  = 0xDEAD;
    GLint stencilBackRef             = 0xDEAD;
    GLuint stencilBackValueMask      = 0xDEAD;
    GLuint stencilBackWriteMask      = 0xDEAD;
    GLenum stencilFrontFail          = 0xDEAD;
    GLenum stencilFrontFunc          = 0xDEAD;
    GLenum stencilFrontPassDepthFail = 0xDEAD;
    GLenum stencilFrontPassDepthPass = 0xDEAD;
    GLint stencilFrontRef            = 0xDEAD;
    GLuint stencilFrontValueMask     = 0xDEAD;
    GLuint stencilFrontWriteMask     = 0xDEAD;
};

class GlGuardRender final {

public:
#define Self GlGuardRender
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self();
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLfloat blendColor[4]     = {0.0f, 0.0f, 0.0f, 0.0f};
    GLenum blendDstAlpha      = 0xDEAD;
    GLenum blendDstRgb        = 0xDEAD;
    GLenum blendEquationAlpha = 0xDEAD;
    GLenum blendEquationRgb   = 0xDEAD;
    GLenum blendSrcAlpha      = 0xDEAD;
    GLenum blendSrcRgb        = 0xDEAD;
    GLfloat clearColor[4]     = {42.42f, 42.42f, 42.42f, 42.42f};
    GLboolean colorMask[4]    = {GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE};
    GLenum cullFaceMode       = 0xDEAD;
    GLenum logicOpMode        = 0xDEAD;
    GLint64 scissor[4]        = {0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD};
    GLint64 viewport[4]       = {0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD};
    GLenum provokingVertex    = 0xDEAD;
};

} // namespace engine::gl