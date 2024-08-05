#pragma once

#include "engine/Prelude.hpp"

namespace engine::gl {

class GlGuardAux final {

public:
#define Self GlGuardAux
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self();
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLenum activeTexture_           = 0xDEAD;
    GLuint program_                 = 0xDEAD;
    GLuint dispatchIndirectBuffer_  = 0xDEAD;
    GLuint drawIndirectBuffer_      = 0xDEAD;
    GLuint programPipeline_         = 0xDEAD;
    GLuint textureBuffer_           = 0xDEAD;
    GLuint transformFeedbackBuffer_ = 0xDEAD;
};

class GlGuardFramebuffer final {

public:
#define Self GlGuardFramebuffer
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self(bool restoreRare);
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLuint drawFramebuffer_         = 0xDEAD;
    GLuint readFramebuffer_         = 0xDEAD;
    GLboolean framebufferSrgb_    = GL_FALSE;
    GLuint pixelPackBuffer_         = 0xDEAD;
    GLuint pixelUnpackBuffer_       = 0xDEAD;
    GLuint renderBuffer_            = 0xDEAD;

    bool restoreRare_ = false;
};

class GlGuardVertex final {

public:
#define Self GlGuardVertex
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self(bool restoreRare);
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLuint vao_                     = 0xDEAD;
    GLuint vbo_                     = 0xDEAD;
    GLuint ebo_                     = 0xDEAD;
    GLboolean primitiveRestart_   = GL_FALSE;
    GLuint primitiveRestartIndex_   = 0xDEAD;
    GLboolean cullFace_           = GL_FALSE;
    GLenum cullFaceMode_       = 0xDEAD;
    GLenum provokingVertex_    = 0xDEAD;

    bool restoreRare_ = false;
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
    GLboolean blend_       = GL_FALSE;
    GLboolean depthTest_   = GL_FALSE;
    GLboolean stencilTest_ = GL_FALSE;
    GLboolean multisample_ = GL_FALSE;
};

class GlGuardColor final {

public:
#define Self GlGuardColor
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self();
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLfloat colorClearValue_[4]     = {42.42f, 42.42f, 42.42f, 42.42f};
    GLboolean colorWriteMask_[4]    = {GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE};
};

class GlGuardDepth final {

public:
#define Self GlGuardDepth
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self(bool restoreRare);
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLboolean depthTest_         = GL_FALSE;
    GLfloat depthClearValue_     = 0.424242f;
    GLenum depthFunc_            = 0xDEAD;
    GLboolean depthWriteMask_         = GL_FALSE;
    GLboolean depthClamp_         = GL_FALSE;
    GLfloat depthRange_[2]       = {42.42f, 42.42f};
    GLboolean polygonOffsetFill_  = GL_FALSE;
    GLboolean polygonOffsetLine_  = GL_FALSE;
    GLboolean polygonOffsetPoint_ = GL_FALSE;
    GLfloat polygonOffsetFactor_ = 0.f;
    GLfloat polygonOffsetUnits_  = 0.f;

    bool restoreRare_ = false;
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
    GLboolean stencilTest_            = GL_FALSE;
    GLint stencilClearValue_          = 0xDEAD;
    GLenum stencilBackFail_           = 0xDEAD;
    GLenum stencilBackFunc_           = 0xDEAD;
    GLenum stencilBackPassDepthFail_  = 0xDEAD;
    GLenum stencilBackPassDepthPass_  = 0xDEAD;
    GLint stencilBackRef_             = 0xDEAD;
    GLuint stencilBackValueMask_      = 0xDEAD;
    GLuint stencilBackWriteMask_      = 0xDEAD;
    GLenum stencilFrontFail_          = 0xDEAD;
    GLenum stencilFrontFunc_          = 0xDEAD;
    GLenum stencilFrontPassDepthFail_ = 0xDEAD;
    GLenum stencilFrontPassDepthPass_ = 0xDEAD;
    GLint stencilFrontRef_            = 0xDEAD;
    GLuint stencilFrontValueMask_     = 0xDEAD;
    GLuint stencilFrontWriteMask_     = 0xDEAD;
};

class GlGuardBlend final {

public:
#define Self GlGuardBlend
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self(bool restoreRare);
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLfloat blendColor_[4]     = {0.0f, 0.0f, 0.0f, 0.0f};
    GLenum blendDstAlpha_      = 0xDEAD;
    GLenum blendDstRgb_        = 0xDEAD;
    GLenum blendEquationAlpha_ = 0xDEAD;
    GLenum blendEquationRgb_   = 0xDEAD;
    GLenum blendSrcAlpha_      = 0xDEAD;
    GLenum blendSrcRgb_        = 0xDEAD;
    // NOTE: GL_LOGIC_OP_MODE and GL_COLOR_LOGIC_OP may go there, but they're very rare features
    GLboolean colorLogicOp_       = GL_FALSE;
    GLenum colorLogicOpMode_        = 0xDEAD;

    bool restoreRare_ = false;
};

class GlGuardViewport final {

public:
#define Self GlGuardViewport
    // NOTE: must only be created on rendering thread, with GL context present
    explicit Self(bool restoreRare);
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    GLboolean scissorTest_        = GL_FALSE;
    GLint64 scissor_[4]        = {0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD};
    GLint64 viewport_[4]       = {0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD};

    bool restoreRare_ = false;
};

} // namespace engine::gl