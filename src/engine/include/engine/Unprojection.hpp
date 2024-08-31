#pragma once

#include "engine/Prelude.hpp"

namespace engine {

struct Frustum final {
    float left;
    float right;
    float bottom;
    float top;
    float near;
    float far;
    Frustum(float left, float right, float bottom, float top, float near, float far)
        : left(left)
        , right(right)
        , bottom(bottom)
        , top(top)
        , near(near)
        , far(far) { }
};

struct FrustumPlanes {
    // each plane represented by coefficients [A, B, C, D], so that Ax + By + Cz + D = 0
    glm::vec4 left;
    glm::vec4 right;
    glm::vec4 bottom;
    glm::vec4 top;
    glm::vec4 near;
    glm::vec4 far;
};

FrustumPlanes CameraToPlanes [[nodiscard]] (glm::mat4 const& mvp, bool normalize = false);
Frustum ProjectionToFrustum [[nodiscard]] (glm::mat4 const& proj);

} // namespace engine
