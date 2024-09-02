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

    void SetPosition(glm::vec3 newPosition) { position_ = newPosition; }
    void SetOrientation(glm::vec3 forward, glm::vec3 up) {
        forwardDirection_ = forward;
        upDirection_ = up;
        SyncInvariant();
    }
    void SetOrientation(glm::vec3 globalEuler) {
        globalEuler.x = glm::clamp(globalEuler.x, -89.0f, 89.0f); // anti gimbal lock

        float cosx = glm::cos(glm::radians(globalEuler.x));
        glm::vec3 direction{
            glm::cos(glm::radians(globalEuler.z)) * cosx,
            -glm::sin(glm::radians(globalEuler.x)),
            glm::sin(glm::radians(globalEuler.z)) * cosx,
        };
        forwardDirection_ = glm::normalize(direction);
        SyncInvariant();
    }

    void MoveGlobally(glm::vec3 direction, float units) { position_ += direction * units; }
    void MoveLocally(glm::vec3 direction, float units) {
        position_ += (direction.x * rightDirection_ + direction.y * upDirection_ + direction.z * forwardDirection_) * units;
    }
    void MoveRight(float units) { MoveGlobally(rightDirection_, units); }
    void MoveUp(float units) { MoveGlobally(upDirection_, units); }
    void MoveForward(float units) { MoveGlobally(forwardDirection_, units); }
private:
    void SyncInvariant() { rightDirection_ = glm::cross(forwardDirection_, upDirection_); }
    glm::vec3 position_{};
    glm::vec3 forwardDirection_{};
    glm::vec3 upDirection_{};
    glm::vec3 rightDirection_{};
};

} // namespace engine