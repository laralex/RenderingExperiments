#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"

#include <memory>

namespace engine::gl {

class VaoMutableCtx;

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
    auto IndexDataType [[nodiscard]] () const -> GLenum { return indexBufferDataType_; }
    auto IndexCount [[nodiscard]] () const -> GLsizei { return indexBufferNumIndices_; }
    auto operator== [[nodiscard]] (Vao const& other) -> bool { return vaoId_ == other.vaoId_; }

private:
    friend class VaoMutableCtx;

    void Dispose();
    GlHandle vaoId_{GL_NONE};

    std::shared_ptr<GpuBuffer> indexBuffer_{};
    GLenum indexBufferDataType_{GL_NONE}; // e.g. GL_UNSIGNED_INT
    GLsizei indexBufferNumIndices_{0};
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
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

private:
    Vao const& contextVao_;
    static bool hasInstances_;
};

class VaoMutableCtx final {

public:
#define Self VaoMutableCtx
    explicit Self(Vao& useVao);
    ~Self()                      = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    auto LinkVertexAttribute
        [[nodiscard]] (GpuBuffer const& attributeBuffer, Vao::AttributeInfo const& info, bool normalized = false)
        -> VaoMutableCtx&&;
    auto LinkIndices [[nodiscard]] (GpuBuffer const& indexBuffer, GLenum dataType) -> VaoMutableCtx&&;

private:
    Vao& contextVao_;
    VaoCtx context_;
};

} // namespace engine::gl
