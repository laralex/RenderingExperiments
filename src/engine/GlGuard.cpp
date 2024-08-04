#include "engine/GlGuard.hpp"

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

GlGuardBindings::GlGuardBindings() {
    GLCALL(glGetIntegerv(GL_ACTIVE_TEXTURE, reinterpret_cast<GLint*>(&activeTexture)));
    GLCALL(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*>(&vbo)));
    GLCALL(glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&program)));
    // NOTE: probably a bad idea to store/restore glDrawBuffers, as it's state of framebuffer
    // for (size_t i = 0; i < sizeof(drawBuffers) / sizeof(drawBuffers[0]); ++i) {
    //     GLCALL(glGetIntegerv(GL_DRAW_BUFFER0 + i, reinterpret_cast<GLint*>(drawBuffers + i)));
    // }
    GLCALL(glGetIntegerv(GL_DISPATCH_INDIRECT_BUFFER_BINDING, reinterpret_cast<GLint*>(&dispatchIndirectBuffer)));
    GLCALL(glGetIntegerv(GL_DRAW_INDIRECT_BUFFER_BINDING, reinterpret_cast<GLint*>(&drawIndirectBuffer)));
    GLCALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&drawFramebuffer)));
    GLCALL(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&readFramebuffer)));
    GLCALL(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*>(&ebo)));
    GLCALL(glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, reinterpret_cast<GLint*>(&pixelPackBuffer)));
    GLCALL(glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, reinterpret_cast<GLint*>(&pixelUnpackBuffer)));
    GLCALL(glGetIntegerv(GL_PRIMITIVE_RESTART_INDEX, reinterpret_cast<GLint*>(&primitiveRestartIndex)));
    GLCALL(glGetIntegerv(GL_PROGRAM_PIPELINE_BINDING, reinterpret_cast<GLint*>(&programPipeline)));
    GLCALL(glGetIntegerv(GL_RENDERBUFFER_BINDING, reinterpret_cast<GLint*>(&renderBuffer)));
    GLCALL(glGetIntegerv(GL_TEXTURE_BUFFER_BINDING, reinterpret_cast<GLint*>(&textureBuffer)));
    GLCALL(glGetIntegerv(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, reinterpret_cast<GLint*>(&transformFeedbackBuffer)));
    GLCALL(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, reinterpret_cast<GLint*>(&vao)));
}

GlGuardBindings::~GlGuardBindings() {
    GLCALL(glActiveTexture(activeTexture));

    GLCALL(glUseProgram(program));
    // TODO: maybe glBindVertexArray(0) and after binding VBO/EBO bind VAO
    GLCALL(glBindVertexArray(vao));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
    GLCALL(glBindProgramPipeline(programPipeline));
    GLCALL(glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, dispatchIndirectBuffer));
    GLCALL(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer));
    GLCALL(glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelPackBuffer));
    GLCALL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelUnpackBuffer));
    GLCALL(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, transformFeedbackBuffer));
    GLCALL(glBindBuffer(GL_TEXTURE_BUFFER, textureBuffer));
    // GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer));

    GLCALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer));
    GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer));
    GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer));
    GLCALL(glPrimitiveRestartIndex(primitiveRestartIndex));
    // GLCALL(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
    // XLOG("~GlGuardBindings", 0);
}

GlGuardFlags::GlGuardFlags() {
    GLCALL(glGetBooleanv(GL_BLEND, &blend));
    GLCALL(glGetBooleanv(GL_DEPTH_TEST, &depthTest));
    GLCALL(glGetBooleanv(GL_MULTISAMPLE, &multisample));
    GLCALL(glGetBooleanv(GL_STENCIL_TEST, &stencilTest));
}

GlGuardFlags::~GlGuardFlags() {
    RestoreFlag(GL_BLEND, blend);
    RestoreFlag(GL_DEPTH_TEST, depthTest);
    RestoreFlag(GL_MULTISAMPLE, multisample);
    RestoreFlag(GL_STENCIL_TEST, stencilTest);
    // XLOG("~GlGuardFlags", 0);
}

GlGuardFlagsRare::GlGuardFlagsRare() {
    GLCALL(glGetBooleanv(GL_COLOR_LOGIC_OP, &colorLogicOp));
    GLCALL(glGetBooleanv(GL_CULL_FACE, &cullFace));
    GLCALL(glGetBooleanv(GL_DEPTH_CLAMP, &depthClamp));
    GLCALL(glGetBooleanv(GL_FRAMEBUFFER_SRGB, &framebufferSrgb));
    GLCALL(glGetBooleanv(GL_POLYGON_OFFSET_FILL, &polygonOffsetFill));
    GLCALL(glGetBooleanv(GL_POLYGON_OFFSET_LINE, &polygonOffsetLine));
    GLCALL(glGetBooleanv(GL_POLYGON_OFFSET_POINT, &polygonOffsetPoint));
    GLCALL(glGetBooleanv(GL_PRIMITIVE_RESTART, &primitiveRestart));
    GLCALL(glGetBooleanv(GL_SCISSOR_TEST, &scissorTest));
}

GlGuardFlagsRare::~GlGuardFlagsRare() {
    RestoreFlag(GL_COLOR_LOGIC_OP, colorLogicOp);
    RestoreFlag(GL_CULL_FACE, cullFace);
    RestoreFlag(GL_DEPTH_CLAMP, depthClamp);
    RestoreFlag(GL_FRAMEBUFFER_SRGB, framebufferSrgb);
    RestoreFlag(GL_POLYGON_OFFSET_FILL, polygonOffsetFill);
    RestoreFlag(GL_POLYGON_OFFSET_LINE, polygonOffsetLine);
    RestoreFlag(GL_POLYGON_OFFSET_POINT, polygonOffsetPoint);
    RestoreFlag(GL_PRIMITIVE_RESTART, primitiveRestart);
    RestoreFlag(GL_SCISSOR_TEST, scissorTest);
    // XLOG("~GlGuardFlagsRare", 0);
}

GlGuardDepth::GlGuardDepth() {
    GLCALL(glGetBooleanv(GL_DEPTH_TEST, &depthTest));
    GLCALL(glGetFloatv(GL_DEPTH_CLEAR_VALUE, &depthClearValue));
    GLCALL(glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&depthFunc)));
    GLCALL(glGetFloatv(GL_DEPTH_RANGE, depthRange));
    GLCALL(glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask));
    GLCALL(glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &polygonOffsetFactor));
    GLCALL(glGetFloatv(GL_POLYGON_OFFSET_UNITS, &polygonOffsetUnits));
}

GlGuardDepth::~GlGuardDepth() {
    RestoreFlag(GL_DEPTH_TEST, depthTest);
    GLCALL(glClearDepth(depthClearValue));
    GLCALL(glDepthFunc(depthFunc));
    GLCALL(glDepthRange(depthRange[0], depthRange[1]));
    GLCALL(glDepthMask(depthMask));
    GLCALL(glPolygonOffset(polygonOffsetFactor, polygonOffsetUnits));
    // XLOG("~GlGuardDepth", 0);
}

GlGuardStencil::GlGuardStencil() {
    GLCALL(glGetBooleanv(GL_DEPTH_TEST, &stencilTest));
    GLCALL(glGetIntegerv(GL_DEPTH_CLEAR_VALUE, &stencilClearValue));

    GLCALL(glGetIntegerv(GL_STENCIL_BACK_FAIL, reinterpret_cast<GLint*>(&stencilBackFail)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_FUNC, reinterpret_cast<GLint*>(&stencilBackFunc)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_FAIL, reinterpret_cast<GLint*>(&stencilBackPassDepthFail)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_PASS, reinterpret_cast<GLint*>(&stencilBackPassDepthPass)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_REF, reinterpret_cast<GLint*>(&stencilBackRef)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_VALUE_MASK, reinterpret_cast<GLint*>(&stencilBackValueMask)));
    GLCALL(glGetIntegerv(GL_STENCIL_BACK_WRITEMASK, reinterpret_cast<GLint*>(&stencilBackWriteMask)));

    GLCALL(glGetIntegerv(GL_STENCIL_FAIL, reinterpret_cast<GLint*>(&stencilFrontFail)));
    GLCALL(glGetIntegerv(GL_STENCIL_FUNC, reinterpret_cast<GLint*>(&stencilFrontFunc)));
    GLCALL(glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, reinterpret_cast<GLint*>(&stencilFrontPassDepthFail)));
    GLCALL(glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, reinterpret_cast<GLint*>(&stencilFrontPassDepthPass)));
    GLCALL(glGetIntegerv(GL_STENCIL_REF, reinterpret_cast<GLint*>(&stencilFrontRef)));
    GLCALL(glGetIntegerv(GL_STENCIL_VALUE_MASK, reinterpret_cast<GLint*>(&stencilFrontValueMask)));
    GLCALL(glGetIntegerv(GL_STENCIL_WRITEMASK, reinterpret_cast<GLint*>(&stencilFrontWriteMask)));
}

GlGuardStencil::~GlGuardStencil() {
    RestoreFlag(GL_DEPTH_TEST, stencilTest);
    GLCALL(glClearStencil(stencilClearValue));
    GLCALL(glStencilOpSeparate(GL_BACK, stencilBackFail, stencilBackPassDepthFail, stencilBackPassDepthPass));
    GLCALL(glStencilFuncSeparate(GL_BACK, stencilBackFunc, stencilBackRef, stencilBackValueMask));
    GLCALL(glStencilMaskSeparate(GL_BACK, stencilBackWriteMask));
    GLCALL(glStencilOpSeparate(GL_FRONT, stencilFrontFail, stencilFrontPassDepthFail, stencilFrontPassDepthPass));
    GLCALL(glStencilFuncSeparate(GL_FRONT, stencilFrontFunc, stencilFrontRef, stencilFrontValueMask));
    GLCALL(glStencilMaskSeparate(GL_FRONT, stencilFrontWriteMask));
    // XLOG("~GlGuardStencil", 0);
}

GlGuardRender::GlGuardRender() {
    GLCALL(glGetFloatv(GL_BLEND_COLOR, blendColor));
    GLCALL(glGetIntegerv(GL_BLEND_DST_ALPHA, reinterpret_cast<GLint*>(&blendDstAlpha)));
    GLCALL(glGetIntegerv(GL_BLEND_DST_RGB, reinterpret_cast<GLint*>(&blendDstRgb)));
    GLCALL(glGetIntegerv(GL_BLEND_SRC_ALPHA, reinterpret_cast<GLint*>(&blendSrcAlpha)));
    GLCALL(glGetIntegerv(GL_BLEND_SRC_RGB, reinterpret_cast<GLint*>(&blendSrcRgb)));
    GLCALL(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, reinterpret_cast<GLint*>(&blendEquationAlpha)));
    GLCALL(glGetIntegerv(GL_BLEND_EQUATION_RGB, reinterpret_cast<GLint*>(&blendEquationRgb)));
    GLCALL(glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor));
    GLCALL(glGetBooleanv(GL_COLOR_WRITEMASK, colorMask));
    GLCALL(glGetIntegerv(GL_CULL_FACE_MODE, reinterpret_cast<GLint*>(&cullFaceMode)));
    GLCALL(glGetIntegerv(GL_LOGIC_OP_MODE, reinterpret_cast<GLint*>(&logicOpMode)));
    GLCALL(glGetInteger64v(GL_SCISSOR_BOX, scissor));
    GLCALL(glGetInteger64v(GL_VIEWPORT, viewport));
    GLCALL(glGetIntegerv(GL_PROVOKING_VERTEX, reinterpret_cast<GLint*>(&provokingVertex)));
}

GlGuardRender::~GlGuardRender() {
    GLCALL(glBlendColor(blendColor[0], blendColor[1], blendColor[2], blendColor[3]));
    GLCALL(glBlendFuncSeparate(blendSrcRgb, blendDstRgb, blendSrcAlpha, blendDstAlpha));
    GLCALL(glBlendEquationSeparate(blendEquationRgb, blendEquationAlpha));
    GLCALL(glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]));
    GLCALL(glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]));
    GLCALL(glCullFace(cullFaceMode));
    GLCALL(glLogicOp(logicOpMode));
    GLCALL(glScissor(scissor[0], scissor[1], scissor[2], scissor[3]));
    GLCALL(glViewport(viewport[0], viewport[1], viewport[2], viewport[3]));
    GLCALL(glProvokingVertex(provokingVertex));
    // XLOG("~GlGuardRender", 0);
}

} // namespace engine::gl