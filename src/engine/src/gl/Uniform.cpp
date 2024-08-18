#include "engine/gl/Uniform.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

bool UniformCtx::hasInstances_{false};
GlHandle UniformCtx::contextProgram_{GL_NONE};

UniformCtx::UniformCtx(GpuProgram const& useProgram) {
    assert(!hasInstances_);
    contextProgram_.id = useProgram.Id();
    GLCALL(glUseProgram(contextProgram_));
    hasInstances_ = true;
}

UniformCtx::~UniformCtx() {
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    contextProgram_.id = GL_NONE;
    GLCALL(glUseProgram(contextProgram_));
    hasInstances_ = false;
}

void UniformCtx::SetUbo(GLuint programBinding, GLuint bufferBindingIdx) const {
    GLCALL(glUniformBlockBinding(contextProgram_.id, programBinding, bufferBindingIdx));
}

void UniformMatrix2(GLint location, GLfloat const* values, GLsizei numMatrices, GLboolean transpose) {
    GLCALL(glUniformMatrix2fv(location, numMatrices, transpose, values));
}

void UniformMatrix3(GLint location, GLfloat const* values, GLsizei numMatrices, GLboolean transpose) {
    GLCALL(glUniformMatrix3fv(location, numMatrices, transpose, values));
}

void UniformMatrix4(GLint location, GLfloat const* values, GLsizei numMatrices, GLboolean transpose) {
    GLCALL(glUniformMatrix4fv(location, numMatrices, transpose, values));
}

void UniformTexture(GLint location, GLint textureSlot) { GLCALL(glUniform1i(location, textureSlot)); }

void UniformBuffer(GpuProgram const& program, GLuint programBinding, GLuint bufferBindingIdx) {
    GLCALL(glUniformBlockBinding(program.Id(), programBinding, bufferBindingIdx));
}

} // namespace engine::gl
