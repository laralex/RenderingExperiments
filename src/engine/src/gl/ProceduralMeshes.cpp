#include "engine/gl/ProceduralMeshes.hpp"
#include "engine/BoxMesh.hpp"
#include "engine/IcosphereMesh.hpp"
#include "engine/PlaneMesh.hpp"
#include "engine/UvSphereMesh.hpp"

namespace {

using namespace engine::gl;

template <typename IndexT> struct AllocateMeshInfo {
    GpuMesh::AttributesLayout layout;
    std::vector<glm::vec3> const& vertexPositions{};
    std::string_view vertexPositionsLabel{};
    void const* vertexData{};
    std::string_view vertexDataLabel{};
    std::vector<IndexT> const& indices{};
    std::string_view indicesLabel{};
    std::string_view vaoLabel{};
    bool isClockwiseWinding;
    GLsizei vertexDataStride;
    GLsizei vertexDataUvOffset;
    GLsizei vertexDataNormalOffset;
};

template <typename IndexT>
auto AllocateMesh [[nodiscard]] (GlContext const& gl, AllocateMeshInfo<IndexT>&& info) -> GpuMesh {
    size_t numVertices = std::size(info.vertexPositions);
    auto positions     = GpuBuffer::Allocate(
        gl, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
        engine::CpuMemory<void const>{info.vertexPositions.data(), numVertices * sizeof(info.vertexPositions[0])},
        info.vertexPositionsLabel);
    auto attributes = GpuBuffer::Allocate(
        gl, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
        engine::CpuMemory<void const>{info.vertexData, numVertices * info.vertexDataStride}, info.vertexDataLabel);
    auto indexBuffer = GpuBuffer::Allocate(
        gl, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
        engine::CpuMemory<void const>{info.indices.data(), std::size(info.indices) * sizeof(info.indices[0])},
        info.indicesLabel);

    auto vao = Vao::Allocate(gl, info.vaoLabel);

    GLenum indexType;
    if constexpr (sizeof(IndexT) == 1) {
        indexType = GL_UNSIGNED_BYTE;
    } else if constexpr (sizeof(IndexT) == 2) {
        indexType = GL_UNSIGNED_SHORT;
    } else if constexpr (sizeof(IndexT) == 4) {
        indexType = GL_UNSIGNED_INT;
    }

    VaoMutableCtx vaoGuard{vao};
    vaoGuard
        .MakeVertexAttribute(
            positions,
            {.location        = info.layout.positionLocation,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(glm::vec3),
             .offset          = 0})
        .MakeIndexed(indexBuffer, indexType);
    if (info.vertexDataUvOffset >= 0) {
        vaoGuard.MakeVertexAttribute(
            attributes,
            {.location        = info.layout.uvLocation,
             .valuesPerVertex = 2,
             .datatype        = GL_FLOAT,
             .stride          = info.vertexDataStride,
             .offset          = info.vertexDataUvOffset});
    }
    if (info.vertexDataUvOffset >= 0) {
        vaoGuard.MakeVertexAttribute(
            attributes,
            {.location        = info.layout.normalLocation,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = info.vertexDataStride,
             .offset          = info.vertexDataNormalOffset});
    }

    GLenum frontFace = info.isClockwiseWinding ? GL_CW : GL_CCW;
    return GpuMesh{std::move(vao),         std::move(positions), std::move(attributes),
                   std::move(indexBuffer), info.layout,          frontFace};
}

} // namespace

namespace engine::gl {

auto AllocateBoxMesh(GlContext const& gl, BoxMesh const& cpuMesh, GpuMesh::AttributesLayout layout) -> GpuMesh {
    return AllocateMesh(
        gl,
        AllocateMeshInfo<uint8_t>{
            .layout                 = layout,
            .vertexPositions        = cpuMesh.vertexPositions,
            .vertexPositionsLabel   = "Box positions VBO",
            .vertexData             = cpuMesh.vertexData.data(),
            .vertexDataLabel        = "Box VBO",
            .indices                = cpuMesh.indices,
            .indicesLabel           = "Box EBO",
            .vaoLabel               = "Box VAO",
            .isClockwiseWinding     = cpuMesh.isClockwiseWinding,
            .vertexDataStride       = sizeof(BoxMesh::Vertex),
            .vertexDataUvOffset     = offsetof(BoxMesh::Vertex, uv),
            .vertexDataNormalOffset = offsetof(BoxMesh::Vertex, normal),
        });
}

auto AllocateIcosphereMesh(GlContext const& gl, IcosphereMesh const& cpuMesh, GpuMesh::AttributesLayout layout)
    -> GpuMesh {
    return AllocateMesh(
        gl,
        AllocateMeshInfo<uint16_t>{
            .layout                 = layout,
            .vertexPositions        = cpuMesh.vertexPositions,
            .vertexPositionsLabel   = "Icosphere positions VBO",
            .vertexData             = cpuMesh.vertexData.data(),
            .vertexDataLabel        = "Icosphere VBO",
            .indices                = cpuMesh.indices,
            .indicesLabel           = "Icosphere EBO",
            .vaoLabel               = "Icosphere VAO",
            .isClockwiseWinding     = cpuMesh.isClockwiseWinding,
            .vertexDataStride       = sizeof(IcosphereMesh::Vertex),
            .vertexDataUvOffset     = offsetof(IcosphereMesh::Vertex, uv),
            .vertexDataNormalOffset = offsetof(IcosphereMesh::Vertex, normal),
        });
}

auto AllocateUvSphereMesh(GlContext const& gl, UvSphereMesh const& cpuMesh, GpuMesh::AttributesLayout layout)
    -> GpuMesh {
    return AllocateMesh(
        gl,
        AllocateMeshInfo<uint16_t>{
            .layout                 = layout,
            .vertexPositions        = cpuMesh.vertexPositions,
            .vertexPositionsLabel   = "UvSphere positions VBO",
            .vertexData             = cpuMesh.vertexData.data(),
            .vertexDataLabel        = "UvSphere VBO",
            .indices                = cpuMesh.indices,
            .indicesLabel           = "UvSphere EBO",
            .vaoLabel               = "UvSphere VAO",
            .isClockwiseWinding     = cpuMesh.isClockwiseWinding,
            .vertexDataStride       = sizeof(UvSphereMesh::Vertex),
            .vertexDataUvOffset     = offsetof(UvSphereMesh::Vertex, uv),
            .vertexDataNormalOffset = offsetof(UvSphereMesh::Vertex, normal),
        });
}

auto AllocatePlaneMesh(GlContext const& gl, PlaneMesh const& cpuMesh, GpuMesh::AttributesLayout layout) -> GpuMesh {
    return AllocateMesh(
        gl,
        AllocateMeshInfo<uint16_t>{
            .layout                 = layout,
            .vertexPositions        = cpuMesh.vertexPositions,
            .vertexPositionsLabel   = "Plane positions VBO",
            .vertexData             = cpuMesh.vertexData.data(),
            .vertexDataLabel        = "Plane VBO",
            .indices                = cpuMesh.indices,
            .indicesLabel           = "Plane EBO",
            .vaoLabel               = "Plane VAO",
            .isClockwiseWinding     = cpuMesh.isClockwiseWinding,
            .vertexDataStride       = sizeof(PlaneMesh::Vertex),
            .vertexDataUvOffset     = offsetof(PlaneMesh::Vertex, uv),
            .vertexDataNormalOffset = -1,
        });
}

} // namespace engine::gl
