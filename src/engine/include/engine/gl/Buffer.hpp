#pragma once

#include "engine/Prelude.hpp"

namespace engine::gl {

class GpuBuffer final {

public:
#define Self GpuBuffer
    explicit Self() = default;
    ~Self() { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto
    Allocate(GLenum targetType, GLenum usage, GLvoid const* data, GLsizeiptr dataSize, std::string_view name = {})
        -> GpuBuffer;
    auto Id [[nodiscard]] () const -> GLuint { return bufferId_; }
    void Fill(GLvoid const* data, GLsizeiptr dataSize) const;

private:
    void Dispose();
    GlHandle bufferId_{GL_NONE};
    GLenum targetType_ = 0xDEAD;
    GLenum usage_      = 0xDEAD;
};

} // namespace engine::gl
