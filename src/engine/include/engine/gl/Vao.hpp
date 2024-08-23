#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"

#include <memory>

namespace engine::gl {

class Vao final {

public:
#define Self Vao
    explicit Self() noexcept = default;
    ~Self() noexcept { Dispose(); };
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
    auto IsInitialized [[nodiscard]] () const -> bool;
    auto IsIndexed [[nodiscard]] () const -> bool { return indexBufferDataType_ != GL_NONE; }
    auto IndexDataType [[nodiscard]] () const -> GLenum { return indexBufferDataType_; }
    auto IndexCount [[nodiscard]] () const -> GLsizei { return numIndices_; }
    auto FirstIndex [[nodiscard]] () const -> GLint { return firstIndex_; }
    auto operator== [[nodiscard]] (Vao const& other) -> bool { return vaoId_ == other.vaoId_; }

private:
    friend class VaoMutableCtx;
    friend class VaoCtx;

    void Dispose();
    GlHandle vaoId_{GL_NONE};

    std::shared_ptr<GpuBuffer> indexBuffer_{};
    GLenum indexBufferDataType_{GL_NONE}; // e.g. GL_UNSIGNED_INT

    // array indices for glDrawArrays, or EBO indices for glDrawElements
    GLsizei numIndices_{0};
    GLint firstIndex_{0};
};

// Helper object, binds GL VAO in ctor, unbinds it in dtor
// No two instances of the class should exist in one scope (checked by assert)

class VaoCtx final {

public:
#define Self VaoCtx
    explicit Self(Vao const& useVao) noexcept;
    ~Self() noexcept;
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
    explicit Self(Vao& useVao) noexcept;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    auto MakeVertexAttribute
        [[nodiscard]] (GpuBuffer const& attributeBuffer, Vao::AttributeInfo const& info, bool normalized = false)
        -> VaoMutableCtx&&;
    auto MakeIndexed [[nodiscard]] (GpuBuffer const& indexBuffer, GLenum dataType, GLint firstVertexId = 0)
    -> VaoMutableCtx&&;
    auto MakeUnindexed [[nodiscard]] (GLsizei numVertexIds, GLint firstVertexId = 0) -> VaoMutableCtx&&;

private:
    Vao& contextVao_;
    VaoCtx context_;
};

} // namespace engine::gl
