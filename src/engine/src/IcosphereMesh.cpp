#include "engine/IcosphereMesh.hpp"

namespace {

constexpr float GOLDEN_RATIO = 1.618033988749;
constexpr float ICO_HEIGHT   = 0.525731112119134;
constexpr float ICO_WIDTH    = ICO_HEIGHT * GOLDEN_RATIO;

constexpr glm::vec3 ICOSAHEDRON_POSITIONS[] = {
    {ICO_WIDTH, ICO_HEIGHT, 0.0f},    // 0
    {0.0f, ICO_WIDTH, -ICO_HEIGHT},   // 1
    {0.0f, ICO_WIDTH, ICO_HEIGHT},    // 2
    {ICO_HEIGHT, -0.0f, -ICO_WIDTH},  // 3
    {ICO_HEIGHT, -0.0f, ICO_WIDTH},   // 4
    {ICO_WIDTH, -ICO_HEIGHT, 0.0f},   // 5
    {-ICO_HEIGHT, 0.0f, -ICO_WIDTH},  // 6
    {-ICO_WIDTH, ICO_HEIGHT, 0.0f},   // 7
    {-ICO_HEIGHT, 0.0f, ICO_WIDTH},   // 8
    {-0.0f, -ICO_WIDTH, -ICO_HEIGHT}, // 9
    {-0.0f, -ICO_WIDTH, ICO_HEIGHT},  // 10
    {-ICO_WIDTH, -ICO_HEIGHT, 0.0f}   // 11
};

// NOTE: counter clock wise winding
constexpr int16_t ICOSAHEDRON_INDICES[] = {0, 1,  2, 0, 3,  1, 0, 2,  4, 3,  0, 5,  0, 4, 5, 1,  3,  6,  1,  7,
                                           2, 7,  1, 6, 4,  2, 8, 7,  8, 2,  9, 3,  5, 6, 3, 9,  5,  4,  10, 4,
                                           8, 10, 9, 5, 10, 7, 6, 11, 7, 11, 8, 11, 6, 9, 8, 11, 10, 10, 11, 9};

using Iter = std::vector<glm::vec3>::iterator;
void Normalize(Iter begin, Iter end) {
    while (begin != end) {
        *begin = glm::normalize(*begin);
        ++begin;
    }
}

auto MakeIcosahedron [[nodiscard]] () -> engine::IcosphereMesh {
    engine::IcosphereMesh mesh;
    mesh.vertexPositions.resize(std::size(ICOSAHEDRON_POSITIONS));
    mesh.vertexPositions.insert(
        mesh.vertexPositions.begin(),
        std::begin(ICOSAHEDRON_POSITIONS), std::end(ICOSAHEDRON_POSITIONS));

    mesh.vertexData.resize(std::size(mesh.vertexPositions));

    mesh.indices.resize(std::size(ICOSAHEDRON_INDICES));
    mesh.indices.insert(
        mesh.indices.begin(),
        std::begin(ICOSAHEDRON_INDICES), std::end(ICOSAHEDRON_INDICES));

    return mesh;
}

} // namespace

namespace engine {

auto IcosphereMesh::Generate(int32_t numSubdivisions, bool clockWiseTriangles) -> IcosphereMesh {
    IcosphereMesh mesh = MakeIcosahedron();
    for (int32_t i = 0; i < numSubdivisions; ++i) {
        // split each triangle into 4
        int32_t const oldNumVerts     = std::size(mesh.vertexPositions);
        int32_t const oldNumIndices   = std::size(mesh.indices);
        int32_t const oldNumTriangles = oldNumIndices / 3;
        int32_t const numVerts        = oldNumVerts + oldNumTriangles * 3;
        int32_t const numIndices      = oldNumIndices * 3; // 4 new triangles: 1 old overwritten, 3 added

        mesh.vertexPositions.reserve(numVerts);
        mesh.vertexData.resize(numVerts);
        mesh.indices.reserve(numIndices);

        for (int32_t t = 0; t < oldNumIndices; t += 3) {
            int32_t const i0   = mesh.indices[t];
            int32_t const i1   = mesh.indices[t + 1];
            int32_t const i2   = mesh.indices[t + 2];
            glm::vec3 const v0 = mesh.vertexPositions[i0];
            glm::vec3 const v1 = mesh.vertexPositions[i1];
            glm::vec3 const v2 = mesh.vertexPositions[i2];

            // new 3 vertices
            int32_t const newIdx = std::size(mesh.vertexPositions);

            // TODO: half of the vertices are duplicated, because each neighboring triangle will generate the same vertices on the edges
            mesh.vertexPositions.emplace_back((v0 + v2) * 0.5f); // newIdx + 0
            mesh.vertexPositions.emplace_back((v0 + v1) * 0.5f); // newIdx + 1
            mesh.vertexPositions.emplace_back((v1 + v2) * 0.5f); // newIdx + 2

            // triangle cornered at v0
            mesh.indices.emplace_back(i0);
            mesh.indices.emplace_back(newIdx + 1);
            mesh.indices.emplace_back(newIdx + 0);

            // triangle cornered at v1
            mesh.indices.emplace_back(i1);
            mesh.indices.emplace_back(newIdx + 2);
            mesh.indices.emplace_back(newIdx + 1);

            // triangle cornered at v2
            mesh.indices.emplace_back(i2);
            mesh.indices.emplace_back(newIdx + 0);
            mesh.indices.emplace_back(newIdx + 2);

            // overwrite old triangle with new inner triangle
            mesh.indices[t]     = newIdx;
            mesh.indices[t + 1] = newIdx + 1;
            mesh.indices[t + 2] = newIdx + 2;
        }
    }

    // normalize positions, compute uv and normals
    for (int32_t i = 0; i < std::size(mesh.vertexPositions); ++i) {
        glm::vec3 position = glm::normalize(mesh.vertexPositions[i]);
        mesh.vertexPositions[i] = position;

        constexpr float PI = glm::pi<float>();
        float u = (glm::atan(position.z,position.x) / (2.0f * PI));
        float v = (glm::asin(position.y) /  PI) + 0.5f;
        mesh.vertexData[i].uv = glm::vec2{u, v};
        mesh.vertexData[i].normal = position;
    }

    if (clockWiseTriangles) {
        InvertTriangleWinding(mesh.indices);
        InvertTriangleNormals(mesh.vertexData.data(), offsetof(Vertex, normal), sizeof(Vertex), std::size(mesh.vertexData));
    }

    mesh.isClockwiseWinding = clockWiseTriangles;
    return mesh;
}

} // namespace engine
