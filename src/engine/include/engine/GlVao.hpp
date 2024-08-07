#pragma once

#include "engine/GlBuffer.hpp"
#include "engine/Prelude.hpp"

namespace engine::gl {

class Vao final {

public:
#define Self Vao
    explicit Self() = default;
    ~Self() { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    struct AttributeInfo {
        GLuint index          = 0U;
        GLint valuesPerVertex = 0U;
        GLenum datatype       = GL_FLOAT;
        GLboolean normalized  = GL_FALSE;
        GLsizei stride        = 0U;
        GLsizei offset        = 0U;
    };

    auto Id [[nodiscard]] () const -> GLuint { return vaoId; }
    void Initialize();
    void DefineVertexAttribute(GpuBuffer const& attributeBuffer, Vao::AttributeInfo const& info);
    void DefineIndices(GpuBuffer const& indexBuffer);

private:
    void Dispose();
    GLuint vaoId = GL_NONE;
};

} // namespace engine::gl
