#include "engine/gl/Vao.hpp"

#include "engine_private/Prelude.hpp"

namespace engine::gl {

ENGINE_STATIC bool VaoCtx::hasInstances_{false};

ENGINE_EXPORT VaoCtx::VaoCtx(Vao const& useVao) noexcept
    : contextVao_(useVao) {
    assert(!hasInstances_);
    GLCALL(glBindVertexArray(contextVao_.vaoId_));
    hasInstances_ = true;
}

ENGINE_EXPORT VaoCtx::~VaoCtx() noexcept {
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    GLCALL(glBindVertexArray(0U));
    hasInstances_ = false;
}

ENGINE_EXPORT VaoMutableCtx::VaoMutableCtx(Vao& useVao) noexcept
    : contextVao_(useVao)
    , context_(useVao) { }

ENGINE_EXPORT void Vao::Dispose() {
    if (vaoId_ == GL_NONE) { return; }
    // LogDebugLabel(*this, "VAO object was disposed");
    XLOG("VAO object was disposed: 0x{:08X}", GLuint(vaoId_));
    GLCALL(glDeleteVertexArrays(1, vaoId_.Ptr()));
    vaoId_.UnsafeReset();
}

ENGINE_EXPORT auto Vao::Allocate(GlContext const& gl, std::string_view name) -> Vao {
    Vao vao{};
    GLCALL(glGenVertexArrays(1, vao.vaoId_.Ptr()));
    GLCALL(glBindVertexArray(vao.vaoId_));
    GLCALL(glBindVertexArray(0U));
    if (!name.empty()) {
        DebugLabel(gl, vao, name);
        LogDebugLabel(gl, vao, "VAO was allocated");
    }
    return vao;
}

ENGINE_EXPORT auto VaoMutableCtx::MakeVertexAttribute(
    GpuBuffer const& attributeBuffer, Vao::AttributeInfo const& info, bool normalized) const -> VaoMutableCtx const& {

    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, attributeBuffer.Id()));

    // NOTE: safe int->ptr cast, because info.offset is of type intptr_t

    auto offset = info.offset;
    for (auto loc = info.location; loc < info.location + info.numLocations; ++loc) {
        auto* offsetPtr = reinterpret_cast<GLsizei*>(offset);
        if (info.datatype == GL_BYTE || info.datatype == GL_UNSIGNED_BYTE || info.datatype == GL_SHORT
            || info.datatype == GL_UNSIGNED_SHORT || info.datatype == GL_INT || info.datatype == GL_UNSIGNED_INT) {
            GLCALL(glVertexAttribIPointer(loc, info.valuesPerVertex, info.datatype, info.stride, offsetPtr));
        } else {
            GLCALL(glVertexAttribPointer(
                loc, info.valuesPerVertex, info.datatype, normalized ? GL_TRUE : GL_FALSE, info.stride, offsetPtr));
        }
        GLCALL(glEnableVertexAttribArray(loc));
        GLCALL(glVertexAttribDivisor(loc, info.instanceDivisor));
        offset += info.offsetAdvance;
    }
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    return *this;
}

ENGINE_EXPORT auto VaoMutableCtx::MakeIndexed(
    GpuBuffer const& indexBuffer, GLenum dataType, GLint indexBufferOffset) const -> VaoMutableCtx const& {
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.Id()));
    GLsizei bytesPerIndex = 1;
    switch (dataType) {
    case GL_UNSIGNED_BYTE:
        bytesPerIndex = 1;
        break;
    case GL_UNSIGNED_SHORT:
        bytesPerIndex = 2;
        break;
    case GL_UNSIGNED_INT:
        bytesPerIndex = 4;
        break;
    default:
        assert(false && "VaoMutableCtx::LinkIndices provided unknown dataType");
        break;
    }
    // TODO:
    // XLOGW("VaoMutableCtx::LinkIndices Not implemented shared_ptr bookeeping", 0);
    assert(
        indexBuffer.SizeBytes() % bytesPerIndex == 0
        && "Index buffer has memory size not divisible by sizeof(dataType)");
    contextVao_.indexBuffer_         = nullptr; // TODO: set provided indexBuffer (pass it as shared_ptr)
    contextVao_.indexBufferDataType_ = dataType;
    contextVao_.numIndices_          = indexBuffer.SizeBytes() / bytesPerIndex;
    contextVao_.firstIndex_          = indexBufferOffset;
    return *this;
}

ENGINE_EXPORT auto VaoMutableCtx::MakeUnindexed(GLsizei numVertexIds, GLint firstVertexId) const
    -> VaoMutableCtx const& {
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0U));
    contextVao_.indexBuffer_         = nullptr; // TODO: set provided indexBuffer (pass it as shared_ptr)
    contextVao_.indexBufferDataType_ = GL_NONE;
    contextVao_.numIndices_          = numVertexIds;
    contextVao_.firstIndex_          = firstVertexId;
    return *this;
}

ENGINE_EXPORT auto Vao::IsInitialized() const -> bool {
    assert(vaoId_ != GL_NONE && "Bad call to engine::gl::Vao methods, object isn't yet initialized");
    return true;
}

} // namespace engine::gl
