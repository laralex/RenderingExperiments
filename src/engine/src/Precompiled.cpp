#include "engine/Precompiled.hpp"

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

namespace engine {

void InvertTriangleStripWinding(std::vector<uint16_t>& triangleIndices) {
    int32_t const numIndices = std::size(triangleIndices);
    for (int32_t t = 0; t < numIndices; t += 2) {
        std::swap(triangleIndices[t], triangleIndices[t + 1]);
    }
}

void InvertTriangleNormals(CpuView<glm::vec3> vertexData) {
    int32_t numVertices = vertexData.NumElements();
    for (int32_t i = 0; i < numVertices; ++i) {
        glm::vec3* normal = vertexData[i];
        *normal *= -1.0f;
    }
}

} // namespace engine
