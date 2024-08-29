#include "engine/PlaneMesh.hpp"

namespace {

// TODO: currently mesh duplicates the 4 vertices for each segment
// can instead use 4 vertices, and use indices to adjust them
constexpr glm::vec3 SEGMENT_POSITIONS[] = {
    {0.0f, 0.0f, 1.0f}, // 0
    {1.0f, 0.0f, 1.0f}, // 1
    {0.0f, 1.0f, 1.0f}, // 2
    {1.0f, 1.0f, 1.0f}, // 3
};

constexpr engine::PlaneMesh::Vertex SEGMENT_DATA[] = {
    {{0.0f, 0.0f}}, // 0
    {{1.0f, 0.0f}}, // 1
    {{0.0f, 1.0f}}, // 2
    {{1.0f, 1.0f}}, // 3
};

// NOTE: counter clock wise winding, GL_TRIANGLE_STRIP
constexpr uint8_t MESH_INDICES[] = {0, 1, 2, 3};

} // namespace

namespace engine {

// based on https://stackoverflow.com/a/5920190/8564999
auto PlaneMesh::Generate(glm::ivec2 numSegments, glm::vec2 uvScale, bool clockwiseTriangles) -> PlaneMesh {
    PlaneMesh mesh;
    constexpr int32_t NUM_SEGMENT_VERTS = 4;
    mesh.vertexPositions.resize(numSegments.x * numSegments.y);
    mesh.vertexData.resize(numSegments.x * numSegments.y);
    mesh.indices.resize(numSegments.x * numSegments.y + (numSegments.x - 1) * (numSegments.y - 2));

    int32_t vertexIdx      = 0;
    int32_t vertexDataIdx  = 0;
    glm::vec3 centerOffset = -0.5f * glm::vec3{numSegments, 0.0f};
    for (int32_t row = 0; row < numSegments.y; ++row) {
        for (int32_t col = 0; col < numSegments.x; ++col) {
            mesh.vertexPositions[vertexIdx] = glm::vec3{static_cast<float>(col), static_cast<float>(row), 0.0f};
            mesh.vertexData[vertexIdx]      = SEGMENT_DATA[vertexDataIdx];
            if (vertexDataIdx >= std::size(SEGMENT_DATA)) { vertexDataIdx = 0; }
            ++vertexIdx;
        }
    }

    int32_t idx = 0;
    for (int row = 0; row < numSegments.y - 1;) {
        for (int col = 0; col < numSegments.x; ++col) {
            mesh.indices[idx++] = col + row * numSegments.x;
            mesh.indices[idx++] = col + (row + 1) * numSegments.x;
        }
        ++row;
        if (row >= numSegments.y - 1) { break; }
        for (int col = numSegments.x - 1; col > 0; --col) {
            mesh.indices[idx++] = col + (row + 1) * numSegments.x;
            mesh.indices[idx++] = col - 1 + +row * numSegments.x;
        }
        ++row;
    }
    if ((numSegments.y & 1) && numSegments.y > 2) { mesh.indices[idx++] = (numSegments.y - 1) * numSegments.x; }

    // adjust triangle winding
    if (clockwiseTriangles) {
        InvertTriangleStripWinding(mesh.indices);
        // InvertTriangleNormals(
        //    mesh.vertexData.data(), offsetof(Vertex, normal), sizeof(Vertex), std::size(mesh.vertexData));
    }

    mesh.isClockwiseWinding = clockwiseTriangles;
    return mesh;
}

} // namespace engine
