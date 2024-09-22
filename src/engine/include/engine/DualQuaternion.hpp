#pragma once

#include <glm/gtc/quaternion.hpp>

namespace engine::math {

auto RotateByQuat(glm::vec3 vertex, glm::vec4 quat) -> glm::vec3{
    //   glm::vec3 uv = glm::cross(quat.xyz, vertex);
    //   glm::vec3 uuv = glm::cross(quat.xyz, uv);
    //   return vertex + ((uv * quat.w) + uuv) * 2.0;
};

struct DualNumber final {
    float real;
    float dual;

    auto Conjugate() const -> DualNumber {
        return DualNumber{
            .real = real,
            .dual = -dual,
        };
    }

    auto Add(DualNumber rhs) -> DualNumber {
        return DualNumber{
            .real = real + rhs.real,
            .dual = dual + rhs.dual,
        };
    }

    auto Mul(DualNumber rhs) -> DualNumber {
        return DualNumber{.real = real * rhs.real, .dual = dual * rhs.real + real * rhs.dual};
    }
};

struct DualQuat final {
    glm::quat real;
    glm::quat dual;

    auto Transform(glm::vec3 v) const -> glm::vec3 { }

    auto DualConjugate() const -> DualQuat {
        return DualQuat{
            .real = real,
            .dual = -dual,
        };
    }

    auto QuatConjugate() const -> DualQuat {
        return DualQuat{
            .real = glm::conjugate(real),
            .dual = glm::conjugate(dual),
        };
    }

    // NOTE: norm is a dual number
    auto MagnitudeSqr() const -> DualNumber {
        return DualNumber{
            .real = MagnitudeReal(),
            .dual = 2.0f * glm::dot(real, dual),
        };
    }

    auto MagnitudeReal() const -> float { return glm::dot(real, real); }

    void Normalize() { Mul(1.0f / MagnitudeReal()); }

    auto IsNormal() const -> bool {
        auto normSqr = MagnitudeSqr();
        return std::abs(normSqr.real - 1.0f) < 0.0001f && std::abs(normSqr.dual - 0.0f) < 0.0001f;
    }

    auto Add(DualQuat rhs) -> DualQuat {
        return DualQuat{
            .real = real + rhs.real,
            .dual = dual + rhs.dual,
        };
    }

    auto Mul(DualQuat rhs) -> DualQuat {
        return DualQuat{
            .real = real * rhs.real,
            .dual = dual * rhs.real + real * rhs.dual,
        };
    }

    auto Mul(float rhs) -> DualQuat {
        return DualQuat{
            .real = real * rhs,
            .dual = dual * rhs,
        };
    }
};

struct DualQuatTransform final {
    // to convert into 4x4 matrix
    // the dual quat should be stored in form
    // q = r + t * epsilon * r
    // https://en.wikipedia.org/wiki/Dual_quaternion#Dual_quaternions_and_4%C3%974_homogeneous_transforms
    glm::quat rotation;
    glm::vec3 translation;

    explicit operator DualQuat() const {
        return DualQuat{
            .real = rotation,
            .dual = glm::quat(0.0f, translation.x, translation.y, translation.z) * rotation,
        };
    }

    auto Transform(glm::vec3 v) const -> glm::vec3 { }
};

} // namespace engine::math