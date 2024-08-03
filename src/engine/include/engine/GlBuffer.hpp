#pragma once

#include "engine/Prelude.hpp"

namespace engine::gl {

class GpuBuffer final {

public:
#define Self GpuBuffer
    explicit Self()              = default;
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    auto Id[[nodiscard]]() const -> GLuint { return bufferId; }
    void Initialize(GLenum targetType, GLenum usage, GLvoid const* data, GLsizeiptr datasize);

private:
    void Dispose();
    GLuint bufferId = GL_NONE;
};

GpuBuffer::~GpuBuffer() {
    Dispose();
}

void GpuBuffer::Dispose() {
    if (bufferId == GL_NONE) { return; }
    glDeleteBuffers(1, &bufferId);
    bufferId = GL_NONE;
}

void GpuBuffer::Initialize(GLenum targetType, GLenum usage, GLvoid const* data, GLsizeiptr datasize) {
    Dispose();
    GLCALL(glGenBuffers(1, &bufferId));
    GLCALL(glBindBuffer(targetType, bufferId));
    GLCALL(glBufferData(targetType, datasize, data, usage));
    GLCALL(glBindBuffer(targetType, 0));
}

} // namespace engine::gl
