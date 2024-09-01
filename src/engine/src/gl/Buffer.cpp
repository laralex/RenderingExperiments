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
    GLenum targetType, GLenum usage, CpuMemory<GLvoid const> data, std::string_view name) -> GpuBuffer {
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
    GLCALL(glBufferData(targetType, data.NumElements(), data[0], usage));
    GLCALL(glBindBuffer(targetType, 0));

    gpuBuffer.targetType_ = targetType;
    gpuBuffer.usage_      = usage;
    gpuBuffer.sizeBytes_  = data.NumElements();

    if (!name.empty()) {
        DebugLabel(gpuBuffer, name);
        LogDebugLabel(gpuBuffer, "GpuBuffer was allocated");
    }
    return gpuBuffer;
}

void GpuBuffer::Fill(CpuMemory<GLvoid const> cpuData, GLintptr gpuByteOffset) const {
    assert(
        usage_ != GL_STATIC_DRAW & usage_ != GL_STATIC_READ & usage_ != GL_STATIC_COPY
        && "Error filling GpuBuffer which was declared STATIC");
    if (bufferId_ == GL_NONE) {
        XLOGE("Error filling not yet initialized GpuBuffer", 0);
        return;
    }
    if (cpuData.IsEmpty()) {
        XLOGW("GpuBuffer::Fill was given empty data", 0);
        return;
    }
    assert(cpuData.NumElements() <= sizeBytes_ && "Error fitting too big data into GpuBuffer allocated storage");
    GLCALL(glBindBuffer(targetType_, bufferId_));
    GLCALL(glBufferSubData(targetType_, gpuByteOffset, cpuData.NumElements(), cpuData[0]));
    GLCALL(glBindBuffer(targetType_, 0));
}

} // namespace engine::gl