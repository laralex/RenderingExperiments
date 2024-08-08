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
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    struct AttributeInfo {
        GLuint index          = 0U;
        GLint valuesPerVertex = 0U;
        GLenum datatype       = GL_FLOAT;
        GLboolean normalized  = GL_FALSE;
        GLsizei stride        = 0U;
        GLsizei offset        = 0U;
    };

    static auto Allocate [[nodiscard]] (std::string_view name = {}) -> Vao;
    auto Id [[nodiscard]] () const -> GLuint { return vaoId_; }
    void LinkVertexAttribute(GpuBuffer const& attributeBuffer, Vao::AttributeInfo const& info) const;
    void LinkIndices(GpuBuffer const& indexBuffer) const;

private:
    void Dispose();
    GlHandle vaoId_{GL_NONE};
};

} // namespace engine::gl
