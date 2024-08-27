#include "engine/IcosphereMesh.hpp"

#include <map>

namespace {

constexpr float GOLDEN_RATIO = 1.618033988749;

constexpr float PHI                         = 1.618033988749; // golden ratio
constexpr glm::vec3 ICOSAHEDRON_POSITIONS[] = {
    {-1.0f, PHI, 0.0},  {1.0f, PHI, 0.0},  {-1.0f, -PHI, 0.0},  {1.0f, -PHI, 0.0},
    {0.0f, -1.0f, PHI}, {0.0f, 1.0f, PHI}, {0.0f, -1.0f, -PHI}, {0.0f, 1.0f, -PHI},
    {PHI, 0.0f, -1.0f}, {PHI, 0.0f, 1.0f}, {-PHI, 0.0f, -1.0f}, {-PHI, 0.0f, 1.0f},
};

// NOTE: counter clock wise winding
constexpr int16_t ICOSAHEDRON_INDICES[] = {
    0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11, 1, 5, 9, 5, 11, 4,  11, 10, 2,  10, 7, 6, 7, 1, 8,
    3, 9,  4, 3, 4, 2, 3, 2, 6, 3, 6, 8,  3, 8,  9,  4, 9, 5, 2, 4,  11, 6,  2,  10, 8,  6, 7, 9, 8, 1,
};
constexpr int16_t NORTH_POLE_IDX = 5;
constexpr int16_t SOUTH_POLE_IDX = 6;
constexpr float PI               = glm::pi<float>();

auto ComputeEquirectangularSphereUv [[nodiscard]] (glm::vec3 unitSpherePosition) -> glm::vec2 {
    float u = (std::atan2(unitSpherePosition.x, unitSpherePosition.z) / (2.0f * PI)) + 0.5f;
    float v = unitSpherePosition.y * 0.5 + 0.5f;
    return {u, v};
}

auto ComputeEquiareaSphereUv [[nodiscard]] (glm::vec3 unitSpherePosition) -> glm::vec2 {
    float u = std::atan2(unitSpherePosition.x, -unitSpherePosition.z) / (2.0f * PI) + 0.5f;
    float v = std::asin(unitSpherePosition.y) / PI + 0.5f;
    return {u, v};
}

auto MakeIcosahedron [[nodiscard]] () -> engine::IcosphereMesh {
    engine::IcosphereMesh mesh;
    mesh.vertexPositions.resize(std::size(ICOSAHEDRON_POSITIONS));
    std::copy(std::begin(ICOSAHEDRON_POSITIONS), std::end(ICOSAHEDRON_POSITIONS), std::begin(mesh.vertexPositions));

    mesh.vertexData.resize(std::size(mesh.vertexPositions));

    mesh.indices.resize(std::size(ICOSAHEDRON_INDICES));
    std::copy(std::begin(ICOSAHEDRON_INDICES), std::end(ICOSAHEDRON_INDICES), std::begin(mesh.indices));

    return mesh;
}

} // namespace

namespace engine {

auto IcosphereMesh::Generate(GenerationArgs args) -> IcosphereMesh {
    IcosphereMesh mesh = MakeIcosahedron();
    for (int32_t i = 0; i < args.numSubdivisions; ++i) {
        // split each triangle into 4
        int32_t const oldNumVerts     = std::size(mesh.vertexPositions);
        int32_t const oldNumIndices   = std::size(mesh.indices);
        int32_t const oldNumTriangles = oldNumIndices / 3;
        int32_t const numVerts        = oldNumVerts + oldNumTriangles * 3;
        int32_t const numIndices      = oldNumIndices * 3; // 4 new triangles: 1 old overwritten, 3 added

        mesh.vertexPositions.reserve(numVerts);
        mesh.vertexData.reserve(numVerts);
        // mesh.indices.reserve(numIndices);

        std::map<std::pair<int32_t, int32_t>, int32_t> vertexBetweenMap;
        // don't create new vertex, if already exists between two given indices
        auto findOrInsertVertex = [&](int32_t idx0, int32_t idx1, glm::vec3 pos) -> int32_t {
            if (idx0 > idx1) { std::swap(idx0, idx1); }
            auto find = vertexBetweenMap.find(std::pair{idx0, idx1});
            if (find != vertexBetweenMap.end()) { return find->second; }
            int32_t newIdx = std::size(mesh.vertexPositions);
            mesh.vertexPositions.emplace_back(pos);
            mesh.vertexData.emplace_back();
            vertexBetweenMap[std::pair{idx0, idx1}] = newIdx;
            return newIdx;
        };

        for (int32_t t = 0; t < oldNumIndices; t += 3) {
            int32_t const i0   = mesh.indices[t];
            int32_t const i1   = mesh.indices[t + 1];
            int32_t const i2   = mesh.indices[t + 2];
            glm::vec3 const v0 = mesh.vertexPositions[i0];
            glm::vec3 const v1 = mesh.vertexPositions[i1];
            glm::vec3 const v2 = mesh.vertexPositions[i2];

            // new vertices
            int32_t newIdx0 = findOrInsertVertex(i0, i2, (v0 + v2) * 0.5f);
            int32_t newIdx1 = findOrInsertVertex(i0, i1, (v0 + v1) * 0.5f);
            int32_t newIdx2 = findOrInsertVertex(i1, i2, (v1 + v2) * 0.5f);

            // triangle cornered at v0
            mesh.indices.emplace_back(i0);
            mesh.indices.emplace_back(newIdx1);
            mesh.indices.emplace_back(newIdx0);

            // triangle cornered at v1
            mesh.indices.emplace_back(i1);
            mesh.indices.emplace_back(newIdx2);
            mesh.indices.emplace_back(newIdx1);

            // triangle cornered at v2
            mesh.indices.emplace_back(i2);
            mesh.indices.emplace_back(newIdx0);
            mesh.indices.emplace_back(newIdx2);

            // overwrite old triangle with new inner triangle
            mesh.indices[t]     = newIdx0;
            mesh.indices[t + 1] = newIdx1;
            mesh.indices[t + 2] = newIdx2;
        }
    }

    // normalize positions, compute uv and normals
    for (int32_t i = 0; i < std::size(mesh.vertexPositions); ++i) {
        glm::vec3 position      = glm::normalize(mesh.vertexPositions[i]);
        mesh.vertexPositions[i] = position;
        mesh.vertexData[i].uv   = ComputeEquiareaSphereUv(position);
        // mesh.vertexData[i].uv     = ComputeEquirectangularSphereUv(position);
        mesh.vertexData[i].normal = position;
    }

    std::unordered_map<int32_t, int32_t> clonedVertices;
    auto findOrInsertVertex = [&](int32_t idx) -> int32_t {
        auto find = clonedVertices.find(idx);
        if (find != clonedVertices.end()) { return find->second; }
        // duplicate
        int32_t newIdx = std::size(mesh.vertexPositions);
        mesh.vertexPositions.emplace_back(mesh.vertexPositions[idx]);
        auto vertexData = mesh.vertexData[idx];
        vertexData.uv.x += 1.0f;
        // mesh.vertexData[idx].uv.x += 1.0f;
        mesh.vertexData.emplace_back(vertexData);
        return newIdx;
    };

    auto fixSeamUv = [&]() {
        for (int32_t t = 0; t < std::size(mesh.indices); t += 3) {
            int32_t const i0   = mesh.indices[t];
            int32_t const i1   = mesh.indices[t + 1];
            int32_t const i2   = mesh.indices[t + 2];
            glm::vec2 const v0 = mesh.vertexData[i0].uv;
            glm::vec2 const v1 = mesh.vertexData[i1].uv;
            glm::vec2 const v2 = mesh.vertexData[i2].uv;

            glm::vec3 v10 = glm::vec3{v1 - v0, 0.0f};
            glm::vec3 v20 = glm::vec3{v2 - v0, 0.0f};
            auto uvOrder  = glm::cross(v10, v20);
            if (uvOrder.z < 0) {
                // not seam
                continue;
            }
            for (int32_t i = 0; i < 3; ++i) {
                auto& idxRef = mesh.indices[t + i];
                if (mesh.vertexData[idxRef].uv.x < 0.25f) { idxRef = findOrInsertVertex(idxRef); }
            }
            XLOG("SEAM {} {} {}", i0, i1, i2);
        }
    };

    if (args.duplicateSeam) { fixSeamUv(); }

    // duplicate pole vertices, to fix UV warping
    auto fixPoleUv = [&](int32_t poleIdx) {
        // find triangles that contain pole
        std::vector<int32_t> poleTriangles;
        int32_t firstPoleIdx = -1;
        glm::vec3 polePosition;
        for (int32_t t = 0; t < std::size(mesh.indices); t += 3) {
            // NOTE: pole is guaranteed to be first index in triangle
            int32_t const idx = mesh.indices[t];
            if (idx != poleIdx) { continue; }
            if (firstPoleIdx < 0) {
                firstPoleIdx = poleIdx;
                continue;
            }
            // duplicate vertex, relink indices
            mesh.indices[t] = std::size(mesh.vertexPositions);
            mesh.vertexPositions.emplace_back(mesh.vertexPositions[firstPoleIdx]);

            // correct U of UV
            auto attribs     = mesh.vertexData[firstPoleIdx];
            auto neighborUv0 = mesh.vertexData[mesh.indices[t + 1]].uv;
            auto neighborUv1 = mesh.vertexData[mesh.indices[t + 2]].uv;
            // attribs.uv.x     = (neighborUv0.x + neighborUv1.x) * 0.5f;
            mesh.vertexData.emplace_back(attribs);
        }
    };

    // if (args.duplicateSeam) {
    //     fixPoleUv(NORTH_POLE_IDX);
    //     fixPoleUv(SOUTH_POLE_IDX);
    // }

    if (args.clockwiseTriangles) {
        InvertTriangleWinding(mesh.indices);
        InvertTriangleNormals(
            mesh.vertexData.data(), offsetof(Vertex, normal), sizeof(Vertex), std::size(mesh.vertexData));
    }

    mesh.isClockwiseWinding = args.clockwiseTriangles;
    return mesh;
}

} // namespace engine
