#pragma once

#include "engine/Precompiled.hpp"

namespace engine {

struct Transform final {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

} // namespace engine