#pragma once

#include "engine/Prelude.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/dual_quaternion.hpp>

namespace engine {

class Camera final {
public:
#define Self Camera
    explicit Self()              = default;
    ~Self()                      = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    auto Projection() const -> glm::mat4 const& { return projection_; }
    auto World2View() const -> glm::mat4 const& { return world2view_; }
    auto ViewToWorld() const -> glm::mat4 const& { return world2view_; }
    auto ViewProjection() const -> glm::mat4 const& { return world2view2projection; }

    // Must be called after finishing all `Set...` methods, in order to `Get...` methods to return up-to-date data
    void CommitChanges();
    void SetPerspective(float verticalFov, float aspect, float zNear = 1.0f, float zFar = 100.0f);
    void SetOrtographic(float xLeft, float xRight, float yBottom, float yTop, float zNear = 1.0f, float zFar = 100.0f);
    void SetPosition(glm::vec3 newPosition);
    void MoveInView(glm::vec3 deltaViewPosition);
    void MoveInWorld(glm::vec3 deltaWorldPosition);
    void RotateInWorld(glm::quat deltaWorldRotation);

private:
    glm::mat4 world2view_;
    glm::mat4 world2view2projection;
    glm::mat4 projection_;
    glm::mat4 view2world_;
    glm::vec3 worldPosition_;
    glm::quat worldRotation_;
    glm::vec3 localAxisX_;
    glm::vec3 localAxisY_;
    glm::vec3 localAxisZ_;
    // float verticalFov_;
    // float aspectRatio_;
    float zNear_;
    float zFar_;

    bool dirty_;
};

} // namespace engine
