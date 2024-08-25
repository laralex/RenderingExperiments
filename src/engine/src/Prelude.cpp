#include "engine/Prelude.hpp"

#include <spdlog/spdlog.h>

namespace engine { 

void InvertTriangleNormals(void* vertexData, int32_t normalsByteOffset, int32_t vertexStride, int32_t numVertices) {
    char* begin = static_cast<char*>(vertexData);
    char* end = begin + numVertices*vertexStride;
    for (char* normals = begin + normalsByteOffset ; normals < end; normals = normals + vertexStride) {
        glm::vec3* normal = reinterpret_cast<glm::vec3*>(normals);
        *normal *= -1.0f;
    }
}

} // namespace engine
