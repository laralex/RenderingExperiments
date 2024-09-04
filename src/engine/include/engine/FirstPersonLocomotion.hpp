#pragma once

#include "engine/Precompiled.hpp"
#include "engine/CommonInterfaces.hpp"
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

    void SetOrientation(glm::quat globalOrient) {
        globalOrientation_ = globalOrient;
        isDirtyOrient_ = true;
    }

    void RotateLocally(glm::quat deltaRotation) {
        globalOrientation_ = deltaRotation * globalOrientation_;
        isDirtyOrient_ = true;
    }

    void MoveGlobally(glm::vec3 direction, float units) { position_ += direction * units; }
    void MoveLocally(glm::vec3 direction, float units) {
        position_ += (direction.x * rightDirection_ + direction.y * upDirection_ + direction.z * forwardDirection_) * units;
    }
    void MoveRight(float units) { MoveGlobally(rightDirection_, units); }
    void MoveUp(float units) { MoveGlobally(upDirection_, units); }
    void MoveForward(float units) { MoveGlobally(forwardDirection_, units); }

private:
    void CommitForwardUpChange() {
        rightDirection_ = glm::cross(forwardDirection_, upDirection_);
        isDirtyForwardUp_ = false;
    }

    void CommitGlobalRotChange() {
        glm::vec3 euler = glm::eulerAngles(globalOrientation_);
        // euler.x = glm::clamp(euler.x, -89.0f, 89.0f); // anti gimbal lock

        float cosx = glm::cos(euler.x);
        glm::vec3 direction{
            glm::cos(euler.z) * cosx,
            -glm::sin(euler.x),
            glm::sin(euler.z) * cosx,
        };

        forwardDirection_ = glm::normalize(direction);
        isDirtyOrient_ = false;
        CommitForwardUpChange();
    }

    glm::vec3 position_{};
    glm::vec3 forwardDirection_{};
    glm::vec3 upDirection_{};
    glm::vec3 rightDirection_{};

    glm::quat globalOrientation_ = glm::identity<glm::quat>();
    bool isDirtyOrient_{false};
    bool isDirtyForwardUp_{false};
};

} // namespace engine