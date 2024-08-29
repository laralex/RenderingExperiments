#pragma once

#include "engine/Prelude.hpp"

namespace engine {

struct PlaneMesh final {
#define Self PlaneMesh
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    struct Vertex final {
        glm::vec2 uv{};
    };

    static auto Generate [[nodiscard]] (
        glm::ivec2 numSegments = glm::ivec2{1, 1}, glm::vec2 uvScale = glm::vec2{1.0f}, bool clockwiseTriangles = false)
    -> PlaneMesh;

    std::vector<glm::vec3> vertexPositions{};
    std::vector<Vertex> vertexData{};
    std::vector<uint16_t> indices{};
    bool isClockwiseWinding = false;
};

} // namespace engine