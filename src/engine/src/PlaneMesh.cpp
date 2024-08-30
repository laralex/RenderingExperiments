#include "engine/PlaneMesh.hpp"

namespace {

constexpr engine::PlaneMesh::Vertex SEGMENT_DATA[] = {
    {{1.0f, 0.0f}}, // 1
    {{0.0f, 0.0f}}, // 0
    {{1.0f, 1.0f}}, // 3
    {{0.0f, 1.0f}}, // 2
};

} // namespace

namespace engine {

// based on https://stackoverflow.com/a/5920190/8564999
auto PlaneMesh::Generate(glm::ivec2 numSegments, glm::vec2 uvScale, bool clockwiseTriangles) -> PlaneMesh {
    constexpr int32_t NUM_SEGMENT_VERTS = 4;
    // numSegments += 1;
    PlaneMesh mesh;
    mesh.vertexPositions.resize(numSegments.x * numSegments.y);
    mesh.vertexData.resize(numSegments.x * numSegments.y);
    mesh.indices.resize(2 * numSegments.x * numSegments.y + 2 * (numSegments.y - 1));

    int32_t vertexIdx      = 0;
    int32_t vertexDataIdx  = 0;
    glm::vec3 centerOffset = -0.5f * (glm::vec3{numSegments - 1, 0.0f});
    uvScale.x *= -1;
    for (int32_t row = 0; row < numSegments.y; ++row) {
        for (int32_t col = 0; col < numSegments.x; ++col) {
            glm::vec3 position = glm::vec3{static_cast<float>(col), static_cast<float>(row), 0.0f};
            mesh.vertexPositions[vertexIdx] = position + centerOffset;
            mesh.vertexData[vertexIdx] = {{glm::vec2{position} * uvScale}};
            if (vertexDataIdx >= std::size(SEGMENT_DATA)) { vertexDataIdx = 0; }
            ++vertexIdx;
        }
    }

    int32_t idx = 0;
    // for (int row = 0; row < numSegments.y - 1;) {
    //     for (int col = 0; col < numSegments.x; ++col) {
    //         mesh.indices[idx++] = col + row * numSegments.x;
    //         mesh.indices[idx++] = col + (row + 1) * numSegments.x;
    //     }
    //     ++row;
    //     if (row >= numSegments.y - 1) { break; }
    //     for (int col = numSegments.x - 1; col > 0; --col) {
    //         mesh.indices[idx++] = col + (row + 1) * numSegments.x;
    //         mesh.indices[idx++] = col - 1 + row * numSegments.x;
    //     }
    //     ++row;
    // }
    // if ((numSegments.y & 1) && numSegments.y > 2) { mesh.indices[idx++] = (numSegments.y - 1) * numSegments.x; }

    for (int32_t row = 0; row < numSegments.y - 1; ++row) {
        mesh.indices[idx++] = row * numSegments.x;
        for (int32_t col = 0; col < numSegments.x; ++col) {
            mesh.indices[idx++] = row * numSegments.x + col;
            mesh.indices[idx++] = (row + 1) * numSegments.x + col;
        }
        mesh.indices[idx++] = (row + 1) * numSegments.x + (numSegments.x - 1);
    }

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
