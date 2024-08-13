#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"

namespace engine::gl {

class VaoCtx;

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
        GLsizei stride        = 0U;
        intptr_t offset       = 0U;
    };

    static auto Allocate [[nodiscard]] (std::string_view name = {}) -> Vao;
    auto Id [[nodiscard]] () const -> GLuint { return vaoId_; }
    auto VerifyInitialization [[nodiscard]] () const -> bool;

private:
    void Dispose();
    GlHandle vaoId_{GL_NONE};
};

// Helper object, binds GL VAO in ctor, unbinds it in dtor
// No two instances of the class should exist in one scope (checked by assert)
class VaoCtx final {
public:
#define Self VaoCtx
    explicit Self(Vao const& useVao);
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    auto LinkVertexAttribute
        [[nodiscard]] (GpuBuffer const& attributeBuffer, Vao::AttributeInfo const& info, bool normalized = false)
        -> VaoCtx&&;
    auto LinkIndices [[nodiscard]] (GpuBuffer const& indexBuffer) -> VaoCtx&&;

private:
    static GlHandle contextVao_;
    static bool hasInstances_;
};

} // namespace engine::gl
