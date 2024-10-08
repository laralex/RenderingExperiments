#include "engine/gl/Guard.hpp"

#include "engine_private/Prelude.hpp"

namespace {

void RestoreFlag(GLenum option, GLboolean shouldEnable) {
    if (shouldEnable) {
        glEnable(option);
    } else {
        glDisable(option);
    }
}

} // namespace

namespace engine::gl {

ENGINE_EXPORT GlGuardAux::GlGuardAux() noexcept {
    GLCALL(glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture_));
    GLCALL(glGetIntegerv(GL_CURRENT_PROGRAM, &program_));
    // NOTE: probably a bad idea to store/restore glDrawBuffers, as it's state of framebuffer
    // for (size_t i = 0; i < std::size(indices); ++i) {
    //     GLCALL(glGetIntegerv(GL_DRAW_BUFFER0 + i, drawBuffers + i)));
    // }
    GLCALL(glGetIntegerv(GL_DISPATCH_INDIRECT_BUFFER_BINDING, &dispatchIndirectBuffer_));
    GLCALL(glGetIntegerv(GL_DRAW_INDIRECT_BUFFER_BINDING, &drawIndirectBuffer_));
    GLCALL(glGetIntegerv(GL_PROGRAM_PIPELINE_BINDING, &programPipeline_));

    // GLCALL(glGetIntegerv(GL_TEXTURE_BUFFER_BINDING, &textureBuffer_));
    GLCALL(glGetIntegerv(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, &transformFeedbackBuffer_));
}

ENGINE_EXPORT GlGuardAux::~GlGuardAux() noexcept {
    GLCALL(glActiveTexture(activeTexture_));

    GLCALL(glUseProgram(program_));
    GLCALL(glBindProgramPipeline(programPipeline_));
    GLCALL(glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, dispatchIndirectBuffer_));
    GLCALL(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer_));
    GLCALL(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, transformFeedbackBuffer_));
    GLCALL(glBindBuffer(GL_TEXTURE_BUFFER, textureBuffer_));
    // GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer));

    // GLCALL(glDrawBuffers(std::size(drawBuffers), drawBuffers));
    // XLOG("~GlGuardAux");
}

ENGINE_EXPORT GlGuardFramebuffer::GlGuardFramebuffer(bool restoreRare) noexcept
    : restoreRare_(restoreRare) {
    GLCALL(glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderBuffer_));
    GLCALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer_));
    if (restoreRare_) {
        GLCALL(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebuffer_));
        GLCALL(glGetBooleanv(GL_FRAMEBUFFER_SRGB, &framebufferSrgb_));
        GLCALL(glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &pixelPackBuffer_));
        GLCALL(glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &pixelUnpackBuffer_));
    }
}

ENGINE_EXPORT GlGuardFramebuffer::~GlGuardFramebuffer() noexcept {
    GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer_));
    GLCALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer_));
    if (restoreRare_) {
        GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer_));
        RestoreFlag(GL_FRAMEBUFFER_SRGB, framebufferSrgb_);
        GLCALL(glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelPackBuffer_));
        GLCALL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelUnpackBuffer_));
    }
}

ENGINE_EXPORT GlGuardVertex::GlGuardVertex(bool restoreRare) noexcept
    : restoreRare_(restoreRare) {
    GLCALL(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao_));
    if (restoreRare_) {
        // NOTE: vbo and ebo are rare to restore, because they should be just once bound to vao
        GLCALL(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo_));
        GLCALL(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ebo_));
        GLCALL(glGetBooleanv(GL_PRIMITIVE_RESTART, &primitiveRestart_));
        GLCALL(glGetIntegerv(GL_PRIMITIVE_RESTART_INDEX, &primitiveRestartIndex_));
        GLCALL(glGetBooleanv(GL_CULL_FACE, &cullFace_));
        GLCALL(glGetIntegerv(GL_CULL_FACE_MODE, &cullFaceMode_));
        GLCALL(glGetIntegerv(GL_PROVOKING_VERTEX, &provokingVertex_));
    }
}

ENGINE_EXPORT GlGuardVertex::~GlGuardVertex() noexcept {
    // TODO: maybe glBindVertexArray(0) and after binding VBO/EBO bind VAO
    GLCALL(glBindVertexArray(vao_));
    if (restoreRare_) {
        GLCALL(glBindBuffer(GL_ARRAY_BUFFER, vbo_));
        GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_));
        RestoreFlag(GL_PRIMITIVE_RESTART, primitiveRestart_);
        GLCALL(glPrimitiveRestartIndex(primitiveRestartIndex_));
        RestoreFlag(GL_CULL_FACE, cullFace_);
        GLCALL(glCullFace(cullFaceMode_));
        GLCALL(glProvokingVertex(provokingVertex_));
    }
    // XLOG("~GlGuardVertex");
}

ENGINE_EXPORT GlGuardFlags::GlGuardFlags() noexcept {
    GLCALL(glGetBooleanv(GL_BLEND, &blend_));
    GLCALL(glGetBooleanv(GL_DEPTH_TEST, &depthTest_));
    GLCALL(glGetBooleanv(GL_MULTISAMPLE, &multisample_));
    GLCALL(glGetBooleanv(GL_STENCIL_TEST, &stencilTest_));
}

ENGINE_EXPORT GlGuardFlags::~GlGuardFlags() noexcept {
    RestoreFlag(GL_BLEND, blend_);
    RestoreFlag(GL_DEPTH_TEST, depthTest_);
    RestoreFlag(GL_MULTISAMPLE, multisample_);
    RestoreFlag(GL_STENCIL_TEST, stencilTest_);
    // XLOG("~GlGuardFlags");
}

ENGINE_EXPORT GlGuardDepth::GlGuardDepth(bool restoreRare) noexcept
    : restoreRare_(restoreRare) {
    GLCALL(glGetBooleanv(GL_DEPTH_TEST, &depthTest_));
    GLCALL(glGetFloatv(GL_DEPTH_CLEAR_VALUE, &depthClearValue_));
    GLCALL(glGetIntegerv(GL_DEPTH_FUNC, &depthFunc_));
    GLCALL(glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteMask_));

    if (restoreRare_) {
        GLCALL(glGetBooleanv(GL_DEPTH_CLAMP, &depthClamp_));
        GLCALL(glGetFloatv(GL_DEPTH_RANGE, depthRange_));
        GLCALL(glGetBooleanv(GL_POLYGON_OFFSET_FILL, &polygonOffsetFill_));
        GLCALL(glGetBooleanv(GL_POLYGON_OFFSET_LINE, &polygonOffsetLine_));
        GLCALL(glGetBooleanv(GL_POLYGON_OFFSET_POINT, &polygonOffsetPoint_));
        GLCALL(glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &polygonOffsetFactor_));
        GLCALL(glGetFloatv(GL_POLYGON_OFFSET_UNITS, &polygonOffsetUnits_));
    }
}

ENGINE_EXPORT GlGuardDepth::~GlGuardDepth() noexcept {
    RestoreFlag(GL_DEPTH_TEST, depthTest_);
    GLCALL(glClearDepth(depthClearValue_));
    GLCALL(glDepthFunc(depthFunc_));
    GLCALL(glDepthMask(depthWriteMask_));

    if (restoreRare_) {
        RestoreFlag(GL_DEPTH_CLAMP, depthClamp_);
        GLCALL(glDepthRange(depthRange_[0], depthRange_[1]));
        RestoreFlag(GL_POLYGON_OFFSET_FILL, polygonOffsetFill_);
        RestoreFlag(GL_POLYGON_OFFSET_LINE, polygonOffsetLine_);
        RestoreFlag(GL_POLYGON_OFFSET_POINT, polygonOffsetPoint_);
        GLCALL(glPolygonOffset(polygonOffsetFactor_, polygonOffsetUnits_));
    }
    // XLOG("~GlGuardDepth");
}

ENGINE_EXPORT GlGuardStencil::GlGuardStencil() noexcept {
    GLCALL(glGetBooleanv(GL_DEPTH_TEST, &stencilTest_));
    GLCALL(glGetIntegerv(GL_DEPTH_CLEAR_VALUE, &stencilClearValue_));

    GLCALL(glGetIntegerv(GL_STENCIL_BACK_FAIL, &stencilBackFail_));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_FUNC, &stencilBackFunc_));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_FAIL, &stencilBackPassDepthFail_));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_PASS, &stencilBackPassDepthPass_));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_REF, &stencilBackRef_));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_VALUE_MASK, &stencilBackValueMask_));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_WRITEMASK, &stencilBackWriteMask_));

    GLCALL(glGetIntegerv(GL_STENCIL_FAIL, &stencilFrontFail_));
    GLCALL(glGetIntegerv(GL_STENCIL_FUNC, &stencilFrontFunc_));
    GLCALL(glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &stencilFrontPassDepthFail_));
    GLCALL(glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &stencilFrontPassDepthPass_));
    GLCALL(glGetIntegerv(GL_STENCIL_REF, &stencilFrontRef_));
    GLCALL(glGetIntegerv(GL_STENCIL_VALUE_MASK, &stencilFrontValueMask_));
    GLCALL(glGetIntegerv(GL_STENCIL_WRITEMASK, &stencilFrontWriteMask_));
}

ENGINE_EXPORT GlGuardStencil::~GlGuardStencil() noexcept {
    RestoreFlag(GL_DEPTH_TEST, stencilTest_);
    GLCALL(glClearStencil(stencilClearValue_));
    GLCALL(glStencilOpSeparate(
        GL_BACK, static_cast<GLenum>(stencilBackFail_), static_cast<GLenum>(stencilBackPassDepthFail_),
        static_cast<GLenum>(stencilBackPassDepthPass_)));
    GLCALL(
        glStencilFuncSeparate(GL_BACK, static_cast<GLenum>(stencilBackFunc_), stencilBackRef_, stencilBackValueMask_));
    GLCALL(glStencilMaskSeparate(GL_BACK, stencilBackWriteMask_));
    GLCALL(glStencilOpSeparate(
        GL_FRONT, static_cast<GLenum>(stencilFrontFail_), static_cast<GLenum>(stencilFrontPassDepthFail_),
        static_cast<GLenum>(stencilFrontPassDepthPass_)));
    GLCALL(glStencilFuncSeparate(
        GL_FRONT, static_cast<GLenum>(stencilFrontFunc_), stencilFrontRef_, stencilFrontValueMask_));
    GLCALL(glStencilMaskSeparate(GL_FRONT, stencilFrontWriteMask_));
    // XLOG("~GlGuardStencil");
}

ENGINE_EXPORT GlGuardBlend::GlGuardBlend(bool restoreRare) noexcept
    : restoreRare_(restoreRare) {
    GLCALL(glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDstAlpha_));
    GLCALL(glGetIntegerv(GL_BLEND_DST_RGB, &blendDstRgb_));
    GLCALL(glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcAlpha_));
    GLCALL(glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrcRgb_));
    GLCALL(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blendEquationAlpha_));
    GLCALL(glGetIntegerv(GL_BLEND_EQUATION_RGB, &blendEquationRgb_));
    if (restoreRare_) {
        GLCALL(glGetFloatv(GL_BLEND_COLOR, blendColor_));
        GLCALL(glGetBooleanv(GL_COLOR_LOGIC_OP, &colorLogicOp_));
        GLCALL(glGetIntegerv(GL_LOGIC_OP_MODE, &colorLogicOpMode_));
    }
}

ENGINE_EXPORT GlGuardBlend::~GlGuardBlend() noexcept {
    GLCALL(glBlendFuncSeparate(blendSrcRgb_, blendDstRgb_, blendSrcAlpha_, blendDstAlpha_));
    GLCALL(glBlendEquationSeparate(blendEquationRgb_, blendEquationAlpha_));
    if (restoreRare_) {
        GLCALL(glBlendColor(blendColor_[0], blendColor_[1], blendColor_[2], blendColor_[3]));
        RestoreFlag(GL_COLOR_LOGIC_OP, colorLogicOp_);
        GLCALL(glLogicOp(colorLogicOpMode_));
    }
    // XLOG("~GlGuardBlend");
}

ENGINE_EXPORT GlGuardViewport::GlGuardViewport(bool restoreRare) noexcept
    : restoreRare_(restoreRare) {
    GLCALL(glGetInteger64v(GL_VIEWPORT, viewport_));
    if (restoreRare_) {
        GLCALL(glGetBooleanv(GL_SCISSOR_TEST, &scissorTest_));
        GLCALL(glGetInteger64v(GL_SCISSOR_BOX, scissor_));
    }
}

ENGINE_EXPORT GlGuardViewport::~GlGuardViewport() noexcept {
    GLCALL(glViewport(viewport_[0], viewport_[1], viewport_[2], viewport_[3]));
    if (restoreRare_) {
        RestoreFlag(GL_SCISSOR_TEST, scissorTest_);
        GLCALL(glScissor(scissor_[0], scissor_[1], scissor_[2], scissor_[3]));
    }
    // XLOG("~GlGuardViewport");
}

ENGINE_EXPORT GlGuardColor::GlGuardColor() noexcept {
    GLCALL(glGetFloatv(GL_COLOR_CLEAR_VALUE, colorClearValue_));
    GLCALL(glGetBooleanv(GL_COLOR_WRITEMASK, colorWriteMask_));
}

ENGINE_EXPORT GlGuardColor::~GlGuardColor() noexcept {
    GLCALL(glClearColor(colorClearValue_[0], colorClearValue_[1], colorClearValue_[2], colorClearValue_[3]));
    GLCALL(glColorMask(colorWriteMask_[0], colorWriteMask_[1], colorWriteMask_[2], colorWriteMask_[3]));

    // XLOG("~GlGuardColor");
}

} // namespace engine::gl