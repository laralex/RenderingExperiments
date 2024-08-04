#pragma once

#include "engine/Prelude.hpp"

namespace engine::gl {

class GpuBuffer final {

public:
#define Self GpuBuffer
    explicit Self() = default;
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    auto Id [[nodiscard]] () const -> GLuint { return bufferId_; }
    void Initialize(GLenum targetType, GLenum usage, GLvoid const* data, GLsizeiptr dataSize);
    void Fill(GLvoid const* data, GLsizeiptr dataSize);

private:
    void Dispose();
    GLuint bufferId_    = GL_NONE;
    GLenum targetType_  = 0xDEAD;
    GLenum usage_       = 0xDEAD;
    bool isInitialized_ = false;
};

GpuBuffer::~GpuBuffer() { Dispose(); }

void GpuBuffer::Dispose() {
    if (bufferId_ == GL_NONE) { return; }
    glDeleteBuffers(1, &bufferId_);
    bufferId_ = GL_NONE;
}

void GpuBuffer::Initialize(GLenum targetType, GLenum usage, GLvoid const* data, GLsizeiptr dataSize) {
    Dispose();
    GLCALL(glGenBuffers(1, &bufferId_));
    GLCALL(glBindBuffer(targetType, bufferId_));
    GLCALL(glBufferData(targetType, dataSize, data, usage));
    GLCALL(glBindBuffer(targetType, 0));
    targetType_    = targetType;
    usage_         = usage;
    isInitialized_ = true;
}

void GpuBuffer::Fill(GLvoid const* data, GLsizeiptr dataSize) {
    if (!isInitialized_) {
        XLOGE("Failed to fill GL buffer, GlBuffer is not initialized", 0);
        return;
    }
    GLCALL(glBindBuffer(targetType_, bufferId_));
    GLCALL(glBufferSubData(targetType_, 0, dataSize, data));
    GLCALL(glBindBuffer(targetType_, 0));
}

} // namespace engine::gl
