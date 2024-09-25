#include "engine/UvSphereMesh.hpp"

#include "engine_private/Prelude.hpp"

namespace {

constexpr float PI = glm::pi<float>();

auto ComputeEquirectangularSphereUv [[nodiscard]] (glm::vec3 unitSpherePosition) -> glm::vec2 {
    float u = (std::atan2(unitSpherePosition.x, unitSpherePosition.z) / (2.0f * PI)) + 0.5f;
    float v = unitSpherePosition.y * 0.5 + 0.5f;
    return {u, v};
}

auto ComputeEquiareaSphereUv [[nodiscard]] (glm::vec3 unitSpherePosition) -> glm::vec2 {
    float u = std::atan2(unitSpherePosition.x, unitSpherePosition.z) / (2.0f * PI) + 0.5f;
    float v = std::asin(unitSpherePosition.y) / PI + 0.5f;
    return {u, v};
}

} // namespace

namespace engine {

// adopted from https://danielsieger.com/blog/2021/03/27/generating-spheres.html
ENGINE_EXPORT auto UvSphereMesh::Generate(GenerationArgs args) -> UvSphereMesh {
    UvSphereMesh mesh;
    if (args.numMeridians <= 2) args.numMeridians = 3;
    if (args.numParallels <= 2) args.numParallels = 3;

    // +2 mean two poles
    mesh.vertexPositions.reserve((args.numMeridians - 1) * args.numParallels + 2);

    // north pole
    int32_t const northPoleIdx = std::size(mesh.vertexPositions);
    mesh.vertexPositions.emplace_back(0.0f, 0.0f, 1.0f);

    // inner vertices
    for (int32_t p = 0; p < args.numParallels - 1; ++p) {
        auto phi = (PI * (p + 1)) / args.numParallels;
        for (int32_t m = 0; m < args.numMeridians; ++m) {
            auto theta = (2.0f * PI * m) / args.numMeridians;
            auto x     = std::sin(phi) * std::cos(theta);
            auto y     = std::sin(phi) * std::sin(theta);
            auto z     = std::cos(phi);
            mesh.vertexPositions.emplace_back(x, y, z);
            mesh.vertexData.emplace_back();
        }
    }

    // south pole
    int32_t const southPoleIdx = std::size(mesh.vertexPositions);
    mesh.vertexPositions.emplace_back(0.0f, 0.0f, -1.0f);

    // triangles for poles
    for (int32_t m = 0; m < args.numMeridians; ++m) {
        auto i0 = m + 1;
        auto i1 = (m + 1) % args.numMeridians + 1;
        mesh.indices.emplace_back(northPoleIdx);
        mesh.indices.emplace_back(i0);
        mesh.indices.emplace_back(i1);
        i0 = m + args.numMeridians * (args.numParallels - 2) + 1;
        i1 = (m + 1) % args.numMeridians + args.numMeridians * (args.numParallels - 2) + 1;
        mesh.indices.emplace_back(southPoleIdx);
        mesh.indices.emplace_back(i1);
        mesh.indices.emplace_back(i0); // TODO: maybe swap with prev line
    }

    // inner triangles
    for (int32_t p = 0; p < args.numParallels - 2; ++p) {
        auto curParallel  = p * args.numMeridians + 1;
        auto nextParallel = (p + 1) * args.numMeridians + 1;
        for (int32_t m = 0; m < args.numMeridians; ++m) {
            auto i0 = curParallel + m;
            auto i1 = curParallel + (m + 1) % args.numMeridians;
            auto i2 = nextParallel + (m + 1) % args.numMeridians;
            auto i3 = nextParallel + m;
            mesh.indices.emplace_back(i0);
            mesh.indices.emplace_back(i3);
            mesh.indices.emplace_back(i1);
            mesh.indices.emplace_back(i3);
            mesh.indices.emplace_back(i2);
            mesh.indices.emplace_back(i1);
        }
    }

    // normalize positions, compute uv and normals
    for (int32_t i = 0; i < std::size(mesh.vertexPositions); ++i) {
        glm::vec3 position      = glm::normalize(mesh.vertexPositions[i]);
        mesh.vertexPositions[i] = position;
        // mesh.vertexData[i].uv   = ComputeEquiareaSphereUv(position);
        mesh.vertexData[i].uv     = ComputeEquirectangularSphereUv(position);
        mesh.vertexData[i].normal = position;
    }

    if (args.clockwiseTriangles) {
        InvertTriangleWinding(mesh.indices);
        InvertTriangleNormals(CpuView<glm::vec3>{
            /* data */ mesh.vertexData.data(),
            /* numElements */ std::size(mesh.vertexData),
            /* byteOffset  */ offsetof(Vertex, normal),
            /* byteStride  */ sizeof(Vertex)});
    }

    mesh.isClockwiseWinding = args.clockwiseTriangles;
    return mesh;
}

} // namespace engine
