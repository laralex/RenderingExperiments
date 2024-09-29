#include "engine/gl/Uniform.hpp"

#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_STATIC bool UniformCtx::hasInstances_{false};
ENGINE_STATIC GlHandle UniformCtx::contextProgram_{GL_NONE};

ENGINE_EXPORT UniformCtx::UniformCtx(GpuProgram const& useProgram) noexcept {
    assert(!hasInstances_ && "Attempt to start a new UniformCtx, while another is alive in the scope");
    contextProgram_.UnsafeAssign(useProgram.programId_);
    GLCALL(glUseProgram(contextProgram_));
    hasInstances_ = true;
}

ENGINE_EXPORT UniformCtx::~UniformCtx() noexcept {
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    contextProgram_.UnsafeReset();
    GLCALL(glUseProgram(0U));
    hasInstances_ = false;
}

ENGINE_EXPORT auto UniformCtx::GetUboLocation(GpuProgram const& program, std::string_view programUboName) -> GLint {
    GLint blockIndex;
    GLCALL(blockIndex = glGetUniformBlockIndex(program.Id(), programUboName.data()));
    return blockIndex;
}

ENGINE_EXPORT auto UniformCtx::GetUboLocation(std::string_view programUboName) const -> GLint {
    GLint blockIndex;
    GLCALL(blockIndex = glGetUniformBlockIndex(contextProgram_, programUboName.data()));
    return blockIndex;
}

ENGINE_EXPORT void UniformCtx::SetUbo(GLuint programLocation, GLuint bufferBinding) const {
    GLCALL(glUniformBlockBinding(contextProgram_, programLocation, bufferBinding));
}

ENGINE_EXPORT void UniformCtx::SetUniformMatrix2x2(
    GLint location, GLfloat const* values, GLsizei numMatrices, GLboolean transpose) {
    GLCALL(glUniformMatrix2fv(location, numMatrices, transpose, values));
}

ENGINE_EXPORT void UniformCtx::SetUniformMatrix3x3(
    GLint location, GLfloat const* values, GLsizei numMatrices, GLboolean transpose) {
    GLCALL(glUniformMatrix3fv(location, numMatrices, transpose, values));
}

ENGINE_EXPORT void UniformCtx::SetUniformMatrix4x4(
    GLint location, GLfloat const* values, GLsizei numMatrices, GLboolean transpose) {
    GLCALL(glUniformMatrix4fv(location, numMatrices, transpose, values));
}

ENGINE_EXPORT void UniformCtx::SetUniformTexture(GLint location, GLint textureSlot) {
    GLCALL(glUniform1i(location, textureSlot));
}

} // namespace engine::gl
