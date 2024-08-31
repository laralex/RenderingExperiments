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

void InvertTriangleNormals(void* vertexData, int32_t normalsByteOffset, int32_t vertexStride, int32_t numVertices) {
    char* begin = static_cast<char*>(vertexData);
    char* end   = begin + numVertices * vertexStride;
    for (char* normals = begin + normalsByteOffset; normals < end; normals = normals + vertexStride) {
        glm::vec3* normal = reinterpret_cast<glm::vec3*>(normals);
        *normal *= -1.0f;
    }
}

} // namespace engine
