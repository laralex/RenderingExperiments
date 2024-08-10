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
    GLint activeTexture_           = 0xDEAD; // GLenum
    GLint program_                 = 0xDEAD; // GLuint
    GLint dispatchIndirectBuffer_  = 0xDEAD; // GLuint
    GLint drawIndirectBuffer_      = 0xDEAD; // GLuint
    GLint programPipeline_         = 0xDEAD; // GLuint
    GLint textureBuffer_           = 0xDEAD; // GLuint
    GLint transformFeedbackBuffer_ = 0xDEAD; // GLuint
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
    GLint drawFramebuffer_    = 0xDEAD; // GLuint
    GLint readFramebuffer_    = 0xDEAD; // GLuint
    GLboolean framebufferSrgb_ = GL_FALSE;
    GLint pixelPackBuffer_    = 0xDEAD; // GLuint
    GLint pixelUnpackBuffer_  = 0xDEAD; // GLuint
    GLint renderBuffer_       = 0xDEAD; // GLuint

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
    GLint vao_                   = 0xDEAD; // GLuint
    GLint vbo_                   = 0xDEAD; // GLuint
    GLint ebo_                   = 0xDEAD; // GLuint
    GLboolean primitiveRestart_   = GL_FALSE;
    GLint primitiveRestartIndex_ = 0xDEAD; // GLuint
    GLboolean cullFace_           = GL_FALSE;
    GLint cullFaceMode_          = 0xDEAD; // GLenum
    GLint provokingVertex_       = 0xDEAD; // GLenum

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
    GLfloat colorClearValue_[4]  = {42.42f, 42.42f, 42.42f, 42.42f};
    GLboolean colorWriteMask_[4] = {GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE};
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
    GLboolean depthTest_          = GL_FALSE;
    GLfloat depthClearValue_      = 0.424242f;
    GLint depthFunc_              = 0xDEAD; // GLenum
    GLboolean depthWriteMask_     = GL_FALSE;
    GLboolean depthClamp_         = GL_FALSE;
    GLfloat depthRange_[2]        = {42.42f, 42.42f};
    GLboolean polygonOffsetFill_  = GL_FALSE;
    GLboolean polygonOffsetLine_  = GL_FALSE;
    GLboolean polygonOffsetPoint_ = GL_FALSE;
    GLfloat polygonOffsetFactor_  = 0.f;
    GLfloat polygonOffsetUnits_   = 0.f;

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
    GLint stencilBackFail_           = 0xDEAD; // GLenum
    GLint stencilBackFunc_           = 0xDEAD; // GLenum
    GLint stencilBackPassDepthFail_  = 0xDEAD; // GLenum
    GLint stencilBackPassDepthPass_  = 0xDEAD; // GLenum
    GLint stencilBackRef_             = 0xDEAD;
    GLint stencilBackValueMask_      = 0xDEAD; // GLuint
    GLint stencilBackWriteMask_      = 0xDEAD; // GLuint
    GLint stencilFrontFail_          = 0xDEAD; // GLenum
    GLint stencilFrontFunc_          = 0xDEAD; // GLenum
    GLint stencilFrontPassDepthFail_ = 0xDEAD; // GLenum
    GLint stencilFrontPassDepthPass_ = 0xDEAD; // GLenum
    GLint stencilFrontRef_            = 0xDEAD;
    GLint stencilFrontValueMask_     = 0xDEAD; // GLenum
    GLint stencilFrontWriteMask_     = 0xDEAD; // GLenum
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
    GLint blendDstAlpha_      = 0xDEAD; // GLenum
    GLint blendDstRgb_        = 0xDEAD; // GLenum
    GLint blendEquationAlpha_ = 0xDEAD; // GLenum
    GLint blendEquationRgb_   = 0xDEAD; // GLenum
    GLint blendSrcAlpha_      = 0xDEAD; // GLenum
    GLint blendSrcRgb_        = 0xDEAD; // GLenum
    // NOTE: GL_LOGIC_OP_MODE and GL_COLOR_LOGIC_OP may go there, but they're very rare features
    GLboolean colorLogicOp_  = GL_FALSE;
    GLint colorLogicOpMode_ = 0xDEAD; // GLenum

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
    GLboolean scissorTest_ = GL_FALSE;
    GLint64 scissor_[4]    = {0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD};
    GLint64 viewport_[4]   = {0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD};

    bool restoreRare_ = false;
};

} // namespace engine::gl