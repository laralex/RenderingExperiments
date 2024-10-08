#include "engine/Precompiled.hpp"

#include "engine_private/Prelude.hpp"

#include <glm/glm.hpp>

namespace engine {

ENGINE_EXPORT void InvertTriangleStripWinding(std::vector<uint16_t>& triangleIndices) {
    int32_t const numIndices = std::size(triangleIndices);
    for (int32_t t = 0; t < numIndices; t += 2) {
        std::swap(triangleIndices[t], triangleIndices[t + 1]);
    }
}

ENGINE_EXPORT void InvertTriangleNormals(CpuView<glm::vec3> vertexData) {
    int32_t numVertices = vertexData.NumElements();
    for (int32_t i = 0; i < numVertices; ++i) {
        glm::vec3* normal = vertexData[i];
        *normal *= -1.0f;
    }
}

ENGINE_EXPORT auto RotateByQuaternion [[nodiscard]] (glm::vec3 v, glm::quat q) -> glm::vec3 {
    glm::vec3 qaxis{q.x, q.y, q.z};
    return v + 2.0f * glm::cross(qaxis, glm::cross(qaxis, v) + q.w * v);
}

} // namespace engine
