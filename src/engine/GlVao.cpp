#include <engine/GlVao.hpp>

namespace engine::gl {

Vao::~Vao() { Dispose(); }

void Vao::Dispose() {
    if (vaoId == GL_NONE) { return; }
    GLCALL(glDeleteVertexArrays(1, &vaoId));
    vaoId = GL_NONE;
}

void Vao::Initialize() {
    Dispose();
    GLCALL(glGenVertexArrays(1, &vaoId));
}

void Vao::DefineVertexAttribute(GpuBuffer const& attributeBuffer, Vao::AttributeInfo const& info) {
    if (vaoId == GL_NONE) {
        XLOGE("Bad call to DefineVertexAttribute, engine::gl::Vao isn't initialized", 0);
        return;
    }

    GLCALL(glBindVertexArray(vaoId));

    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, attributeBuffer.Id()));

    GLCALL(glVertexAttribPointer(
        info.index, info.valuesPerVertex, info.datatype, info.normalized, info.stride,
        reinterpret_cast<GLsizei*>(info.offset)));
    GLCALL(glEnableVertexAttribArray(info.index));

    GLCALL(glBindVertexArray(0));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Vao::DefineIndices(GpuBuffer const& indexBuffer) {
    if (vaoId == GL_NONE) {
        XLOGE("Bad call to DefineIndices, engine::gl::Vao isn't initialized", 0);
        return;
    }

    GLCALL(glBindVertexArray(vaoId));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.Id()));
    GLCALL(glBindVertexArray(0));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

} // namespace engine::gl
