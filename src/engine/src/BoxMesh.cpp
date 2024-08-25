#include "engine/BoxMesh.hpp"

namespace {

constexpr glm::vec3 VERTEX_POSITIONS[] = {
    {-1.0f, -1.0f, 1.0f},  // 0
    {1.0f, -1.0f, 1.0f},   // 1
    {1.0f, 1.0f, 1.0f},    // 2
    {-1.0f, 1.0f, 1.0f},   // 3
    {-1.0f, 1.0f, -1.0f},  // 4
    {1.0f, 1.0f, -1.0f},   // 5
    {1.0f, -1.0f, -1.0f},  // 6
    {-1.0f, -1.0f, -1.0f}, // 7

    {-1.0f, -1.0f, 1.0f},  // 8 (dup 0-7)
    {1.0f, -1.0f, 1.0f},   // 9
    {1.0f, 1.0f, 1.0f},    // 10
    {-1.0f, 1.0f, 1.0f},   // 11
    {-1.0f, 1.0f, -1.0f},  // 12
    {1.0f, 1.0f, -1.0f},   // 13
    {1.0f, -1.0f, -1.0f},  // 14
    {-1.0f, -1.0f, -1.0f}, // 15

    {-1.0f, -1.0f, 1.0f},  // 16 (dup 0-7)
    {1.0f, -1.0f, 1.0f},   // 17
    {1.0f, 1.0f, 1.0f},    // 18
    {-1.0f, 1.0f, 1.0f},   // 19
    {-1.0f, 1.0f, -1.0f},  // 20
    {1.0f, 1.0f, -1.0f},   // 21
    {1.0f, -1.0f, -1.0f},  // 22
    {-1.0f, -1.0f, -1.0f}, // 23
};

constexpr engine::BoxMesh::Vertex VERTEX_DATA[] = {
    {{0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // 0
    {{1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // 1
    {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},  // 2
    {{0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},  // 3
    {{0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // 4
    {{1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // 5
    {{1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}, // 6
    {{0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}, // 7
    {{0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}}, // 8
    {{1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // 9
    {{1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},  // 10
    {{0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}}, // 11
    {{0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}}, // 12
    {{1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // 13
    {{1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},  // 14
    {{0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}}, // 15
    {{0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}}, // 16
    {{1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}}, // 17
    {{1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},  // 18
    {{0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},  // 19
    {{0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // 20
    {{1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // 21
    {{1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}}, // 22
    {{0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}}, // 23
};

constexpr uint32_t Z_POS0 = 0, Z_POS1 = 1, Z_POS2 = 2, Z_POS3 = 3;
constexpr uint32_t Z_NEG0 = 4, Z_NEG1 = 5, Z_NEG2 = 6, Z_NEG3 = 7;
constexpr uint32_t X_POS0 = 10, X_POS1 = 9, X_POS2 = 14, X_POS3 = 13;
constexpr uint32_t X_NEG0 = 11, X_NEG1 = 12, X_NEG2 = 15, X_NEG3 = 8;
constexpr uint32_t Y_POS0 = 18, Y_POS1 = 21, Y_POS2 = 20, Y_POS3 = 19;
constexpr uint32_t Y_NEG0 = 23, Y_NEG1 = 22, Y_NEG2 = 17, Y_NEG3 = 16;

constexpr uint8_t MESH_INDICES[] = {
    Z_POS0, Z_POS1, Z_POS2, Z_POS0, Z_POS2, Z_POS3, Z_NEG0, Z_NEG1, Z_NEG2, Z_NEG0, Z_NEG2, Z_NEG3,

    X_POS0, X_POS1, X_POS2, X_POS0, X_POS2, X_POS3, X_NEG0, X_NEG1, X_NEG2, X_NEG0, X_NEG2, X_NEG3,

    Y_POS0, Y_POS1, Y_POS2, Y_POS0, Y_POS2, Y_POS3, Y_NEG0, Y_NEG1, Y_NEG2, Y_NEG0, Y_NEG2, Y_NEG3,
};

} // namespace

namespace engine {

auto BoxMesh::Generate(glm::vec3 bakedScale) -> BoxMesh {
    BoxMesh mesh;
    int32_t numVerts = std::size(VERTEX_POSITIONS);
    mesh.vertexPositions.resize(numVerts);
    mesh.vertexData.resize(numVerts);
    for (auto i = 0; i < numVerts; ++i) {
        mesh.vertexPositions[i] = VERTEX_POSITIONS[i] * bakedScale;
        mesh.vertexData[i]      = VERTEX_DATA[i];

        glm::vec3 perpendicular = glm::abs(VERTEX_DATA[i].normal);
        glm::vec2 uvScale;
        if (VERTEX_DATA[i].normal.x != 0.0f) {
            // yz plane
            uvScale = glm::vec2{bakedScale.y, bakedScale.z};
        } else if (VERTEX_DATA[i].normal.y != 0.0f) {
            // xz plane
            uvScale = glm::vec2{bakedScale.x, bakedScale.z};
        } else if (VERTEX_DATA[i].normal.z != 0.0f) {
            // xy plane
            uvScale = glm::vec2{bakedScale.x, bakedScale.y};
        }
        mesh.vertexData[i].uv *= uvScale;
    }

    mesh.indices.resize(std::size(MESH_INDICES));
    std::copy(std::begin(MESH_INDICES), std::end(MESH_INDICES), mesh.indices.begin());
    return mesh;
}

} // namespace engine
