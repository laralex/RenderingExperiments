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

} // namespace engine::gl
