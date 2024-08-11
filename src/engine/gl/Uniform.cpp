#include "engine/gl/Uniform.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

bool UniformCtx::hasInstances = false;

UniformCtx::UniformCtx(GpuProgram const& useProgram) {
    assert(!hasInstances);
    GLCALL(glUseProgram(useProgram.Id()));
    hasInstances = true;
}

UniformCtx::~UniformCtx() {
    assert(hasInstances);
    GLCALL(glUseProgram(0U));
    hasInstances = false;
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

} // namespace engine::gl
