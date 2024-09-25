#include "engine/Camera.hpp"

#include "engine_private/Prelude.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace engine {

ENGINE_EXPORT void Camera::SetPerspective(float verticalFov, float aspect, float zNear, float zFar) {
    projection_ = glm::perspective(verticalFov, aspect, zNear, zFar);

    // verticalFov_ = verticalFov;
    // aspectRatio_ = aspect;
    zNear_ = zNear;
    zFar_  = zFar;
    dirty_ = true;
}

ENGINE_EXPORT void Camera::SetOrtographic(float xLeft, float xRight, float yBottom, float yTop, float zNear, float zFar) {
    projection_ = glm::ortho(xLeft, xRight, yBottom, yTop, zNear, zFar);
    zNear_      = zNear;
    zFar_       = zFar;
    dirty_      = true;
}

ENGINE_EXPORT void Camera::SetPosition(glm::vec3 newPosition) {
    worldPosition_ = newPosition;
    dirty_         = true;
}

ENGINE_EXPORT void Camera::MoveInView(glm::vec3 deltaViewPosition) {
    worldPosition_ += deltaViewPosition;
    dirty_ = true;
}

ENGINE_EXPORT void Camera::MoveInWorld(glm::vec3 deltaWorldPosition) {
    worldPosition_ += deltaWorldPosition;
    dirty_ = true;
}

ENGINE_EXPORT void Camera::RotateInWorld(glm::quat deltaWorldRotation) {
    worldRotation_ = deltaWorldRotation * worldRotation_;
    dirty_         = true;
}

ENGINE_EXPORT void Camera::CommitChanges() {
    if (!dirty_) { return; }
    world2view_           = glm::lookAtLH(worldPosition_, forwardAxis_, upAxis_);
    view2world_           = glm::inverse(world2view_);
    world2view2projection = projection_ * world2view_;
    dirty_                = false;
}

} // namespace engine
