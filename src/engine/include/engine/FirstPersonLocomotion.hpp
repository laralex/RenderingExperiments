#pragma once

#include "engine/Precompiled.hpp"
#include "engine/CommonInterfaces.hpp"
#include <glm/gtc/quaternion.hpp>

namespace engine {

class FirstPersonLocomotion final : public IDirty {

public:
#define Self FirstPersonLocomotion
    explicit Self()              = default;
    virtual ~Self() override {}
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto ComputeViewMatrix[[nodiscard]](glm::vec3 const& position, glm::vec3 const& forward, glm::vec3 const& up) -> glm::mat4 {
        // TODO: in theory can construct matrix in place, without extra calculation from 3 vectors we already have
        return glm::lookAtRH(position, position+forward, up);
    }

    virtual void CommitChanges() override {
        if (isDirtyOrient_) {
            CommitGlobalRotChange();
        } else if (isDirtyForwardUp_) {
            CommitForwardUpChange();
        }
    }

    auto ComputeViewMatrix [[nodiscard]]() const -> glm::mat4 { return ComputeViewMatrix(position_, forwardDirection_, upDirection_); }
    auto ComputeModelMatrix [[nodiscard]]() const -> glm::mat4 { return glm::inverse(ComputeViewMatrix(position_, forwardDirection_, upDirection_)); }

    void Clone(FirstPersonLocomotion const& other) {
        std::memcpy((void*)this, (void*)&other, sizeof(FirstPersonLocomotion));
    }

    auto Position[[nodiscard]]() const -> glm::vec3 { return position_; }
    auto Forward[[nodiscard]]() const -> glm::vec3 { return forwardDirection_; }
    auto Up[[nodiscard]]() const -> glm::vec3 { return upDirection_; }

    void SetPosition(glm::vec3 newPosition) { position_ = newPosition; }
    void SetOrientation(glm::vec3 forward, glm::vec3 up) {
        forwardDirection_ = forward;
        upDirection_ = up;
        isDirtyForwardUp_ = true;
    }

    // void SetOrientation(glm::quat globalOrient) {
    //     globalOrientation_ = globalOrient;
    //     isDirtyOrient_ = true;
    // }

    void RotateLocally(glm::vec2 deltaYawPitchRadians) {
        if (deltaYawPitchRadians.x == 0.0f & deltaYawPitchRadians.y == 0.0f) {
            return;
        }
        glm::quat yawRot = glm::angleAxis(deltaYawPitchRadians.x, VEC_UP);
        glm::quat pitchRot = 
        // glm::identity<glm::quat>();
        glm::angleAxis(deltaYawPitchRadians.y, yawRot * VEC_RIGHT);
        globalOrientation_ = glm::normalize(pitchRot * globalOrientation_ * yawRot);
        forwardDirection_ = globalOrientation_ * VEC_FORWARD;
        // forwardDirection_ * glm::mat3_cast(deltaRotation);
        glm::vec3 localRight = glm::cross(forwardDirection_, VEC_UP);
        upDirection_ = glm::cross(localRight, forwardDirection_);
        // auto& q = globalOrientation_;
        // XLOG("xyz {} {} {} w {} len {}", q.x, q.y, q.z, q.w, glm::length(q));
        isDirtyForwardUp_ = true;
    }

    void MoveGlobally(glm::vec3 direction) { position_ += direction; }
    void MoveLocally(glm::vec3 direction) {
        if (direction.x != 0.0f) {
            position_ += direction.x * glm::cross(forwardDirection_, upDirection_);
        }
        position_ += direction.y * upDirection_ + direction.z * forwardDirection_;
    }
    void MoveRight(float units) { MoveGlobally(glm::cross(forwardDirection_, upDirection_) * units); }
    void MoveUp(float units) { MoveGlobally(VEC_UP * units); }
    void MoveForward(float units) { MoveGlobally(forwardDirection_ * units); }

private:
    void CommitForwardUpChange() {
        if (isDirtyForwardUp_) {
            isDirtyForwardUp_ = false;
        }
    }

    void CommitGlobalRotChange() {
        if (isDirtyOrient_) {
            // glm::vec3 euler = glm::eulerAngles(globalOrientation_);
            // // euler.x = glm::clamp(euler.x, -89.0f, 89.0f); // anti gimbal lock
            // glm::vec
            // float cosx = glm::cos(euler.x);
            // glm::vec3 direction{
            //     glm::cos(euler.z) * cosx,
            //     -glm::sin(euler.x),
            //     glm::sin(euler.z) * cosx,
            // };


            isDirtyForwardUp_ = true;
            CommitForwardUpChange();
            isDirtyOrient_ = false;
        }
    }

    glm::vec3 position_{};
    glm::vec3 forwardDirection_{};
    glm::vec3 upDirection_{};

    glm::quat globalOrientation_ = glm::identity<glm::quat>();
    bool isDirtyOrient_{false};
    bool isDirtyForwardUp_{false};
};

} // namespace engine