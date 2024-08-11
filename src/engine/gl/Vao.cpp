#include "engine/gl/Vao.hpp"

namespace engine::gl {

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

void Vao::LinkVertexAttribute(GpuBuffer const& attributeBuffer, Vao::AttributeInfo const& info) const {
    if (vaoId_ == GL_NONE) {
        XLOGE("Bad call to LinkVertexAttribute, engine::gl::Vao isn't initialized", 0);
        return;
    }

    GLCALL(glBindVertexArray(vaoId_));

    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, attributeBuffer.Id()));

    // NOTE: safe int->ptr cast, because info.offset is of type intptr_t
    auto* offset = reinterpret_cast<GLsizei*>(info.offset);
    GLCALL(
        glVertexAttribPointer(info.index, info.valuesPerVertex, info.datatype, info.normalized, info.stride, offset));
    GLCALL(glEnableVertexAttribArray(info.index));

    GLCALL(glBindVertexArray(0));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Vao::LinkIndices(GpuBuffer const& indexBuffer) const {
    if (vaoId_ == GL_NONE) {
        XLOGE("Bad call to LinkIndices, engine::gl::Vao isn't initialized", 0);
        return;
    }

    GLCALL(glBindVertexArray(vaoId_));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.Id()));
    GLCALL(glBindVertexArray(0));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

} // namespace engine::gl
