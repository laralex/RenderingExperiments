#pragma once

#include "engine/Prelude.hpp"

namespace engine {

struct BoxMesh final {
#define Self BoxMesh
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

    static auto Generate(glm::vec3 bakedScale = glm::vec3{1.0f}, bool clockwiseTriangles = false) -> BoxMesh;

    std::vector<glm::vec3> vertexPositions{};
    std::vector<Vertex> vertexData{};
    std::vector<uint8_t> indices{};
    bool isClockwiseWinding = false;
};

} // namespace engine