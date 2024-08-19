#pragma once

#include <cassert>
#include <glad/gl.h>

namespace engine {
    template<typename T>
    struct CpuView;
} // namespace engine

namespace engine::gl {

struct ShaderDefine;

auto LinkProgram(std::string_view vertexShaderCode, std::string_view fragmentShaderCode, std::string_view name = {}) -> std::optional<GpuProgram>;
auto LinkProgramFromFiles(std::string_view vertexFilepath, std::string_view fragmentFilepath, CpuView<ShaderDefine const> defines, std::string_view name = {}) -> std::optional<GpuProgram>;

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
    // Self& operator=(Self&& other) = delete;
    Self& operator=(Self&& other) {
        assert(id == GL_NONE && "OpenGL resource leaked");
        id       = other.id;
        other.id = GL_NONE;
        return *this;
    }
#undef Self
    operator GLuint() const { return id; }
    GLuint id;
};

} // namespace engine::gl