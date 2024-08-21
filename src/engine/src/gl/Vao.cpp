#include "engine/gl/Vao.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

bool VaoCtx::hasInstances_{false};

VaoCtx::VaoCtx(Vao const& useVao) : contextVao_(useVao) {
    assert(!hasInstances_);
    GLCALL(glBindVertexArray(contextVao_.vaoId_));
    hasInstances_ = true;
}

VaoCtx::~VaoCtx() {
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    GLCALL(glBindVertexArray(0U));
    hasInstances_ = false;
}

VaoMutableCtx::VaoMutableCtx(Vao& useVao) : contextVao_(useVao), context_(useVao) {}


void Vao::Dispose() {
    if (vaoId_ == GL_NONE) { return; }
    LogDebugLabel(*this, "VAO object was disposed");
    GLCALL(glDeleteVertexArrays(1, &vaoId_));
    vaoId_.UnsafeReset();
}

auto Vao::Allocate(std::string_view name) -> Vao {
    Vao vao{};
    GLCALL(glGenVertexArrays(1, &vao.vaoId_));
    GLCALL(glBindVertexArray(vao.vaoId_));
    GLCALL(glBindVertexArray(0U));
    if (!name.empty()) {
        DebugLabel(vao, name);
        LogDebugLabel(vao, "VAO was allocated");
    }
    return vao;
}

auto VaoMutableCtx::LinkVertexAttribute(GpuBuffer const& attributeBuffer, Vao::AttributeInfo const& info, bool normalized)
    -> VaoMutableCtx&& {

    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, attributeBuffer.Id()));

    // NOTE: safe int->ptr cast, because info.offset is of type intptr_t
    auto* offset = reinterpret_cast<GLsizei*>(info.offset);
    if (info.datatype == GL_BYTE || info.datatype == GL_UNSIGNED_BYTE || info.datatype == GL_SHORT
        || info.datatype == GL_UNSIGNED_SHORT || info.datatype == GL_INT || info.datatype == GL_UNSIGNED_INT) {
        GLCALL(glVertexAttribIPointer(info.index, info.valuesPerVertex, info.datatype, info.stride, offset));
    } else {
        GLCALL(glVertexAttribPointer(
            info.index, info.valuesPerVertex, info.datatype, normalized ? GL_TRUE : GL_FALSE, info.stride, offset));
    }
    GLCALL(glEnableVertexAttribArray(info.index));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    return std::move(*this);
}

auto VaoMutableCtx::LinkIndices(GpuBuffer const& indexBuffer, GLenum dataType) -> VaoMutableCtx&& {
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.Id()));
    contextVao_.indexBuffer_ = nullptr; // TODO: set provided indexBuffer (pass it as shared_ptr)
    contextVao_.indexBufferDataType_ = dataType;
    XLOGW("VaoMutableCtx::LinkIndices Not implemented shared_ptr bookeeping", 0);
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
    assert (indexBuffer.SizeBytes() % bytesPerIndex == 0 && "Index buffer has memory size not divisible by sizeof(dataType)");
    contextVao_.indexBufferNumIndices_ = indexBuffer.SizeBytes() / bytesPerIndex;
    return std::move(*this);
}

auto VaoMutableCtx::LinkIndices [[nodiscard]] (GLsizei numVirtualIndices) -> VaoMutableCtx&& {
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0U));
    contextVao_.indexBuffer_ = nullptr; // TODO: set provided indexBuffer (pass it as shared_ptr)
    contextVao_.indexBufferDataType_ = GL_UNSIGNED_INT;
    contextVao_.indexBufferNumIndices_ = numVirtualIndices;
    return std::move(*this);
}

auto Vao::VerifyInitialization() const -> bool {
    assert(vaoId_ != GL_NONE && "Bad call to engine::gl::Vao methods, object isn't yet initialized");
    return true;
}

} // namespace engine::gl
