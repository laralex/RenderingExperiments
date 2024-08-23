#include <engine/gl/Buffer.hpp>
#include <engine/gl/Debug.hpp>

namespace engine::gl {

void GpuBuffer::Dispose() {
    if (bufferId_ == GL_NONE) { return; }
    LogDebugLabel(*this, "GpuBuffer was disposed");
    glDeleteBuffers(1, &bufferId_);
    bufferId_.UnsafeReset();
}

auto GpuBuffer::Allocate(
    GLenum targetType, GLenum usage, GLvoid const* data, GLsizeiptr dataSize, std::string_view name) -> GpuBuffer {
    {
        GLenum t = targetType;
        assert(
            t == GL_ARRAY_BUFFER || t == GL_ATOMIC_COUNTER_BUFFER || t == GL_COPY_READ_BUFFER
            || t == GL_COPY_WRITE_BUFFER || t == GL_DISPATCH_INDIRECT_BUFFER || t == GL_DRAW_INDIRECT_BUFFER
            || t == GL_ELEMENT_ARRAY_BUFFER || t == GL_PIXEL_PACK_BUFFER || t == GL_PIXEL_UNPACK_BUFFER
            || t == GL_QUERY_BUFFER || t == GL_SHADER_STORAGE_BUFFER || t == GL_TEXTURE_BUFFER
            || t == GL_UNIFORM_BUFFER);
    }
    GpuBuffer gpuBuffer{};
    GLCALL(glGenBuffers(1, &gpuBuffer.bufferId_));
    GLCALL(glBindBuffer(targetType, gpuBuffer.bufferId_));
    GLCALL(glBufferData(targetType, dataSize, data, usage));
    GLCALL(glBindBuffer(targetType, 0));

    gpuBuffer.targetType_ = targetType;
    gpuBuffer.usage_      = usage;
    gpuBuffer.sizeBytes_  = dataSize;

    if (!name.empty()) {
        DebugLabel(gpuBuffer, name);
        LogDebugLabel(gpuBuffer, "GpuBuffer was allocated");
    }
    return gpuBuffer;
}

void GpuBuffer::Fill(GLvoid const* data, GLsizeiptr dataSize) const {
    assert(dataSize <= sizeBytes_ && "Provided data is too big for GpuBuffer allocated storage");
    if (bufferId_ == GL_NONE) {
        XLOGE("Failed to fill GL buffer, GlBuffer is not initialized", 0);
        return;
    }
    GLCALL(glBindBuffer(targetType_, bufferId_));
    GLCALL(glBufferSubData(targetType_, 0, dataSize, data));
    GLCALL(glBindBuffer(targetType_, 0));
}

} // namespace engine::gl