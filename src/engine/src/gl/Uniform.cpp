#include "engine/gl/Uniform.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

bool UniformCtx::hasInstances_{false};
GlHandle UniformCtx::contextProgram_{GL_NONE};

UniformCtx::UniformCtx(GpuProgram const& useProgram) noexcept {
    assert(!hasInstances_ && "Attempt to start a new UniformCtx, while another is alive in the scope");
    contextProgram_.UnsafeAssign(useProgram.programId_);
    GLCALL(glUseProgram(contextProgram_));
    hasInstances_ = true;
}

UniformCtx::~UniformCtx() noexcept {
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    contextProgram_.UnsafeReset();
    GLCALL(glUseProgram(0U));
    hasInstances_ = false;
}

void UniformCtx::SetUbo(GLuint programBinding, GLuint bufferBindingIdx) const {
    GLCALL(glUniformBlockBinding(contextProgram_, programBinding, bufferBindingIdx));
}

void UniformCtx::SetUniformMatrix2(GLint location, GLfloat const* values, GLsizei numMatrices, GLboolean transpose) {
    GLCALL(glUniformMatrix2fv(location, numMatrices, transpose, values));
}

void UniformCtx::SetUniformMatrix3(GLint location, GLfloat const* values, GLsizei numMatrices, GLboolean transpose) {
    GLCALL(glUniformMatrix3fv(location, numMatrices, transpose, values));
}

void UniformCtx::SetUniformMatrix4(GLint location, GLfloat const* values, GLsizei numMatrices, GLboolean transpose) {
    GLCALL(glUniformMatrix4fv(location, numMatrices, transpose, values));
}

void UniformCtx::SetUniformTexture(GLint location, GLint textureSlot) { GLCALL(glUniform1i(location, textureSlot)); }

} // namespace engine::gl
