#pragma once

#include "engine/Precompiled.hpp"

namespace engine {

struct FirstPersonLocomotion final {

public:
#define Self FirstPersonLocomotion
    explicit Self()              = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto ComputeViewMatrix[[nodiscard]](glm::vec3 const& position, glm::vec3 const& forward, glm::vec3 const& up) -> glm::mat4 {
        // TODO: in theory can construct matrix in place, without extra calculation from 3 vectors we already have
        return glm::lookAtRH(position, position+forward, up);
    }
    auto ComputeViewMatrix [[nodiscard]]() const -> glm::mat4 { return ComputeViewMatrix(position_, forwardDirection_, upDirection_); }
    auto ComputeModelMatrix [[nodiscard]]() const -> glm::mat4 { return glm::inverse(ComputeViewMatrix(position_, forwardDirection_, upDirection_)); }

    auto Clone(FirstPersonLocomotion const& other) {
        std::memcpy(this, &other, sizeof(FirstPersonLocomotion));
    }

    auto Position[[nodiscard]]() const -> glm::vec3 { return position_; }
    auto Forward[[nodiscard]]() const -> glm::vec3 { return forwardDirection_; }
    auto Up[[nodiscard]]() const -> glm::vec3 { return upDirection_; }

    void SetPosition(glm::vec3 newPosition) { position_ = newPosition; }
    void SetOrientation(glm::vec3 forward, glm::vec3 up) {
        forwardDirection_ = forward;
        upDirection_ = up;
        SyncRight();
    }
    void SetOrientation(glm::vec3 globalEuler) {
        globalRotationEuler_ = globalEuler;
        SyncOrientation();
    }

    void RotateLocally(glm::vec3 deltaEuler) {
        globalRotationEuler_ += deltaEuler;
        SyncOrientation();
    }

    void MoveGlobally(glm::vec3 direction, float units) { position_ += direction * units; }
    void MoveLocally(glm::vec3 direction, float units) {
        position_ += (direction.x * rightDirection_ + direction.y * upDirection_ + direction.z * forwardDirection_) * units;
    }
    void MoveRight(float units) { MoveGlobally(rightDirection_, units); }
    void MoveUp(float units) { MoveGlobally(upDirection_, units); }
    void MoveForward(float units) { MoveGlobally(forwardDirection_, units); }

private:
    void SyncRight() { rightDirection_ = glm::cross(forwardDirection_, upDirection_); }
    void SyncOrientation() {
        globalRotationEuler_.x = glm::clamp(globalRotationEuler_.x, -89.0f, 89.0f); // anti gimbal lock

        float cosx = glm::cos(glm::radians(globalRotationEuler_.x));
        glm::vec3 direction{
            glm::cos(glm::radians(globalRotationEuler_.z)) * cosx,
            -glm::sin(glm::radians(globalRotationEuler_.x)),
            glm::sin(glm::radians(globalRotationEuler_.z)) * cosx,
        };
        forwardDirection_ = glm::normalize(direction);
        SyncRight();
    }

    glm::vec3 position_{};
    glm::vec3 forwardDirection_{};
    glm::vec3 upDirection_{};
    glm::vec3 rightDirection_{};

    glm::vec3 globalRotationEuler_{};
};

} // namespace engine