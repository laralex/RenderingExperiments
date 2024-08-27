#pragma once

#include "engine/Prelude.hpp"

namespace engine {

struct IcosphereMesh final {
#define Self IcosphereMesh
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    struct Vertex final {
        glm::vec2 uv{};
        glm::vec3 normal{};
    };

    struct GenerationArgs final {
        int32_t numSubdivisions = 3;
        bool duplicateSeam      = true;
        bool clockwiseTriangles = false;
    };
    static auto Generate(GenerationArgs args) -> IcosphereMesh;

    std::vector<glm::vec3> vertexPositions{};
    std::vector<Vertex> vertexData{};
    std::vector<uint16_t> indices{};
    bool isClockwiseWinding = false;
};

} // namespace engine