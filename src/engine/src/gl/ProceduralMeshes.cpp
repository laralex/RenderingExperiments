#include "engine/gl/ProceduralMeshes.hpp"
#include "engine/BoxMesh.hpp"
#include "engine/IcosphereMesh.hpp"
#include "engine/UvSphereMesh.hpp"

namespace { } // namespace

namespace engine::gl {

auto AllocateBoxMesh(BoxMesh const& cpuMesh, GpuMesh::AttributesLayout layout) -> GpuMesh {
    auto positions = GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, cpuMesh.vertexPositions.data(),
        std::size(cpuMesh.vertexPositions) * sizeof(cpuMesh.vertexPositions[0]), "Box positions VBO");
    auto attributes = GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, cpuMesh.vertexData.data(),
        std::size(cpuMesh.vertexData) * sizeof(cpuMesh.vertexData[0]), "Box VBO");
    auto indexBuffer = GpuBuffer::Allocate(
        GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, cpuMesh.indices.data(),
        std::size(cpuMesh.indices) * sizeof(cpuMesh.indices[0]), "Box EBO");

    auto vao = Vao::Allocate("Box VAO");

    (void)VaoMutableCtx{vao}
        .MakeVertexAttribute(
            positions,
            {.index           = layout.positionLocation,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(glm::vec3),
             .offset          = 0})
        .MakeVertexAttribute(
            attributes,
            {.index           = layout.uvLocation,
             .valuesPerVertex = 2,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(BoxMesh::Vertex),
             .offset          = offsetof(BoxMesh::Vertex, uv)})
        .MakeVertexAttribute(
            attributes,
            {.index           = layout.normalLocation,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(BoxMesh::Vertex),
             .offset          = offsetof(BoxMesh::Vertex, normal)})
        .MakeIndexed(indexBuffer, GL_UNSIGNED_BYTE);

    GLenum frontFace = cpuMesh.isClockwiseWinding ? GL_CW : GL_CCW;
    return GpuMesh{std::move(vao), std::move(positions), std::move(attributes), std::move(indexBuffer), layout,
                   frontFace};
}

auto AllocateIcosphereMesh(IcosphereMesh const& cpuMesh, GpuMesh::AttributesLayout layout) -> GpuMesh {
    auto positions = GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, cpuMesh.vertexPositions.data(),
        std::size(cpuMesh.vertexPositions) * sizeof(cpuMesh.vertexPositions[0]), "Icosphere positions VBO");
    auto attributes = GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, cpuMesh.vertexData.data(),
        std::size(cpuMesh.vertexData) * sizeof(cpuMesh.vertexData[0]), "Icosphere VBO");
    auto indexBuffer = GpuBuffer::Allocate(
        GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, cpuMesh.indices.data(),
        std::size(cpuMesh.indices) * sizeof(cpuMesh.indices[0]), "Icosphere EBO");

    auto vao = Vao::Allocate("Icosphere VAO");

    (void)VaoMutableCtx{vao}
        .MakeVertexAttribute(
            positions,
            {.index           = layout.positionLocation,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(glm::vec3),
             .offset          = 0})
        .MakeVertexAttribute(
            attributes,
            {.index           = layout.uvLocation,
             .valuesPerVertex = 2,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(IcosphereMesh::Vertex),
             .offset          = offsetof(IcosphereMesh::Vertex, uv)})
        .MakeVertexAttribute(
            attributes,
            {.index           = layout.normalLocation,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(IcosphereMesh::Vertex),
             .offset          = offsetof(IcosphereMesh::Vertex, normal)})
        .MakeIndexed(indexBuffer, GL_UNSIGNED_SHORT);

    GLenum frontFace = cpuMesh.isClockwiseWinding ? GL_CW : GL_CCW;
    return GpuMesh{std::move(vao), std::move(positions), std::move(attributes), std::move(indexBuffer), layout,
                   frontFace};
}

auto AllocateUvSphereMesh(UvSphereMesh const& cpuMesh, GpuMesh::AttributesLayout layout) -> GpuMesh {
    auto positions = GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, cpuMesh.vertexPositions.data(),
        std::size(cpuMesh.vertexPositions) * sizeof(cpuMesh.vertexPositions[0]), "UvSphere positions VBO");
    auto attributes = GpuBuffer::Allocate(
        GL_ARRAY_BUFFER, GL_STATIC_DRAW, cpuMesh.vertexData.data(),
        std::size(cpuMesh.vertexData) * sizeof(cpuMesh.vertexData[0]), "UvSphere VBO");
    auto indexBuffer = GpuBuffer::Allocate(
        GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, cpuMesh.indices.data(),
        std::size(cpuMesh.indices) * sizeof(cpuMesh.indices[0]), "UvSphere EBO");

    auto vao = Vao::Allocate("UvSphere VAO");

    (void)VaoMutableCtx{vao}
        .MakeVertexAttribute(
            positions,
            {.index           = layout.positionLocation,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(glm::vec3),
             .offset          = 0})
        .MakeVertexAttribute(
            attributes,
            {.index           = layout.uvLocation,
             .valuesPerVertex = 2,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(UvSphereMesh::Vertex),
             .offset          = offsetof(UvSphereMesh::Vertex, uv)})
        .MakeVertexAttribute(
            attributes,
            {.index           = layout.normalLocation,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(UvSphereMesh::Vertex),
             .offset          = offsetof(UvSphereMesh::Vertex, normal)})
        .MakeIndexed(indexBuffer, GL_UNSIGNED_SHORT);

    GLenum frontFace = cpuMesh.isClockwiseWinding ? GL_CW : GL_CCW;
    return GpuMesh{std::move(vao), std::move(positions), std::move(attributes), std::move(indexBuffer), layout,
                   frontFace};
}

} // namespace engine::gl
