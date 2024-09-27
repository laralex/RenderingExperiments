#include <engine/gl/Buffer.hpp>
#include <engine/gl/Debug.hpp>

#include "engine/gl/Extensions.hpp"
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
    GlContext const& gl, GLenum targetType, Access access, CpuMemory<GLvoid const> data, std::string_view name)
    -> GpuBuffer {
    {
        GLenum t = targetType;
        assert(
            t == GL_ARRAY_BUFFER || t == GL_ATOMIC_COUNTER_BUFFER || t == GL_COPY_READ_BUFFER
            || t == GL_COPY_WRITE_BUFFER || t == GL_DISPATCH_INDIRECT_BUFFER || t == GL_DRAW_INDIRECT_BUFFER
            || t == GL_ELEMENT_ARRAY_BUFFER || t == GL_PIXEL_PACK_BUFFER || t == GL_PIXEL_UNPACK_BUFFER
            || /* t == GL_QUERY_BUFFER_EXT || */ t == GL_SHADER_STORAGE_BUFFER || t == GL_TEXTURE_BUFFER
            || t == GL_UNIFORM_BUFFER);
    }
    GpuBuffer gpuBuffer{};
    GLCALL(glGenBuffers(1, gpuBuffer.bufferId_.Ptr()));
    GLCALL(glBindBuffer(targetType, gpuBuffer.bufferId_));
    if (gl.Extensions().Supports(GlExtensions::ARB_buffer_storage)) {
        /* GL_MAP_READ_BIT, GL_MAP_WRITE_BIT, GL_MAP_PERSISTENT_BIT, GL_MAP_COHERENT_BIT, and GL_CLIENT_STORAGE_BIT */
        GLbitfield flags = ((access & CLIENT_UPDATE) ? GL_DYNAMIC_STORAGE_BIT : GL_NONE)
            | ((access & CLIENT_READ) ? GL_MAP_READ_BIT : GL_NONE);
        GLCALL(glBufferStorage(targetType, data.NumElements(), data[0], flags));
    } else {
        // GL_[STREAM/DYNAMIC/STATIC]_[DRAW/READ/COPY]
        GLenum usage = GL_NONE;
        if (access & CLIENT_UPDATE) {
            usage = (access & CLIENT_READ) ? GL_DYNAMIC_READ : GL_DYNAMIC_DRAW;
        } else {
            usage = (access & CLIENT_READ) ? GL_STATIC_READ : GL_STATIC_DRAW;
        }
        GLCALL(glBufferData(targetType, data.NumElements(), data[0], usage));
    }
    GLCALL(glBindBuffer(targetType, 0));

    gpuBuffer.targetType_ = targetType;
    gpuBuffer.accessMask_      = access;
    gpuBuffer.sizeBytes_  = data.NumElements();

    if (!name.empty()) {
        DebugLabel(gl, gpuBuffer, name);
        LogDebugLabel(gl, gpuBuffer, "GpuBuffer was allocated");
    }
    return gpuBuffer;
}

ENGINE_EXPORT void GpuBuffer::Fill(CpuMemory<GLvoid const> cpuData, GLintptr gpuByteOffset) const {
    assert((accessMask_ & CLIENT_UPDATE) && "Error filling GpuBuffer which was declared STATIC");
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
