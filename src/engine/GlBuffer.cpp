#include <engine/GlBuffer.hpp>
#include <engine/GlDebug.hpp>

namespace engine::gl {

void GpuBuffer::Dispose() {
    if (bufferId_ == GL_NONE) { return; }
    LogDebugLabel(*this, "GpuBuffer was disposed");
    glDeleteBuffers(1, &bufferId_.id);
    bufferId_.id = GL_NONE;
}

auto GpuBuffer::Allocate(
    GLenum targetType, GLenum usage, GLvoid const* data, GLsizeiptr dataSize, std::string_view name) -> GpuBuffer {
    GpuBuffer gpuBuffer{};
    GLCALL(glGenBuffers(1, &gpuBuffer.bufferId_.id));
    GLCALL(glBindBuffer(targetType, gpuBuffer.bufferId_));
    GLCALL(glBufferData(targetType, dataSize, data, usage));
    GLCALL(glBindBuffer(targetType, 0));
    gpuBuffer.targetType_ = targetType;
    gpuBuffer.usage_      = usage;
    if (!name.empty()) {
        DebugLabel(gpuBuffer, name);
        LogDebugLabel(gpuBuffer, "GpuBuffer was allocated");
    }
    return gpuBuffer;
}

void GpuBuffer::Fill(GLvoid const* data, GLsizeiptr dataSize) const {
    if (bufferId_ == GL_NONE) {
        XLOGE("Failed to fill GL buffer, GlBuffer is not initialized", 0);
        return;
    }
    GLCALL(glBindBuffer(targetType_, bufferId_));
    GLCALL(glBufferSubData(targetType_, 0, dataSize, data));
    GLCALL(glBindBuffer(targetType_, 0));
}

} // namespace engine::gl