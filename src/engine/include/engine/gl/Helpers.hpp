#pragma once

#include "engine/Prelude.hpp"

namespace engine::gl {

void InitializeOpenGl();

// Wrapper for OpenGL object identifiers. Becomes 0 when moved away from
// This helps to define move constructor/assignment of other high level wrappers as simply "=default"
struct GlHandle {
#define Self GlHandle
    explicit Self(GLuint id)
        : id(id) { }
    ~Self()                      = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&& other)
        : id(other.id) {
        other.id = GL_NONE;
    }
    Self& operator=(Self&& other) {
        id       = other.id;
        other.id = GL_NONE;
        return *this;
    }
#undef Self
    operator GLuint() const { return id; }
    GLuint id;
};

} // namespace engine::gl