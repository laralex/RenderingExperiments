#include "engine/gl/Vao.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

bool VaoCtx::hasInstances_{false};
GlHandle VaoCtx::contextVao_{GL_NONE};

VaoCtx::VaoCtx(Vao const& useVao) {
    assert(!hasInstances_);
    contextVao_.id = useVao.Id();
    GLCALL(glBindVertexArray(contextVao_));
    hasInstances_ = true;
}

VaoCtx::~VaoCtx() {
    assert(hasInstances_);
    contextVao_.id = GL_NONE;
    GLCALL(glBindVertexArray(contextVao_));
    hasInstances_ = false;
}

void Vao::Dispose() {
    if (vaoId_ == GL_NONE) { return; }
    LogDebugLabel(*this, "VAO object was disposed");
    GLCALL(glDeleteVertexArrays(1, &vaoId_.id));
    vaoId_.id = GL_NONE;
}

auto Vao::Allocate(std::string_view name) -> Vao {
    Vao vao{};
    GLCALL(glGenVertexArrays(1, &vao.vaoId_.id));
    GLCALL(glBindVertexArray(vao.vaoId_));
    GLCALL(glBindVertexArray(0U));
    if (!name.empty()) {
        DebugLabel(vao, name);
        LogDebugLabel(vao, "VAO was allocated");
    }
    return vao;
}

auto VaoCtx::LinkVertexAttribute(GpuBuffer const& attributeBuffer, Vao::AttributeInfo const& info, bool normalized)
    -> VaoCtx&& {

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

auto VaoCtx::LinkIndices(GpuBuffer const& indexBuffer) -> VaoCtx&& {
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.Id()));
    return std::move(*this);
}

auto Vao::VerifyInitialization() const -> bool {
    assert(vaoId_ != GL_NONE && "Bad call to engine::gl::Vao methods, object isn't yet initialized");
    return true;
}

} // namespace engine::gl
