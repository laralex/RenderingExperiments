#pragma once

#include "engine/Prelude.hpp"

namespace engine {

struct UvSphereMesh final {
#define Self UvSphereMesh
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
        int32_t numMeridians    = 10;
        int32_t numParallels    = 10;
        bool duplicateSeam      = true;
        bool clockWiseTriangles = false;
    };
    static auto Generate(GenerationArgs args) -> UvSphereMesh;

    std::vector<glm::vec3> vertexPositions{};
    std::vector<Vertex> vertexData{};
    std::vector<uint16_t> indices{};
    bool isClockwiseWinding = false;
};

} // namespace engine