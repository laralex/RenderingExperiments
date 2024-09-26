#include <engine/gl/Buffer.hpp>
#include <engine/gl/Debug.hpp>

#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_EXPORT void GpuBuffer::Dispose() {
    if (bufferId_ == GL_NONE) { return; }
    // LogDebugLabel(*this, "GpuBuffer was disposed");
    XLOG("GpuBuffer was disposed: 0x{:08X}", GLuint(bufferId_));
    glDeleteBuffers(1, bufferId_.Ptr());
    bufferId_.UnsafeReset();
}

ENGINE_EXPORT auto GpuBuffer::Allocate(
    GlContext const& gl, GLenum targetType, GLenum usage, CpuMemory<GLvoid const> data, std::string_view name)
    -> GpuBuffer {
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
    GLCALL(glGenBuffers(1, gpuBuffer.bufferId_.Ptr()));
    GLCALL(glBindBuffer(targetType, gpuBuffer.bufferId_));
    GLCALL(glBufferData(targetType, data.NumElements(), data[0], usage));
    //TODO glBufferStorageEXT
    assert(false);
    GLCALL(glBindBuffer(targetType, 0));

    gpuBuffer.targetType_ = targetType;
    gpuBuffer.usage_      = usage;
    gpuBuffer.sizeBytes_  = data.NumElements();

    if (!name.empty()) {
        DebugLabel(gl, gpuBuffer, name);
        LogDebugLabel(gl, gpuBuffer, "GpuBuffer was allocated");
    }
    return gpuBuffer;
}

ENGINE_EXPORT void GpuBuffer::Fill(CpuMemory<GLvoid const> cpuData, GLintptr gpuByteOffset) const {
    assert(
        usage_ != GL_STATIC_DRAW & usage_ != GL_STATIC_READ & usage_ != GL_STATIC_COPY
        && "Error filling GpuBuffer which was declared STATIC");
    if (bufferId_ == GL_NONE) {
        XLOGE("Error filling not yet initialized GpuBuffer");
        return;
    }
    if (cpuData.IsEmpty()) {
        XLOGW("GpuBuffer::Fill was given empty data");
        return;
    }
    assert(cpuData.NumElements() <= sizeBytes_ && "Error fitting too big data into GpuBuffer allocated storage");
    GLCALL(glBindBuffer(targetType_, bufferId_));
    GLCALL(glBufferSubData(targetType_, gpuByteOffset, cpuData.NumElements(), cpuData[0]));
    GLCALL(glBindBuffer(targetType_, 0));
}

} // namespace engine::gl
