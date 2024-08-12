#pragma once

#include "engine/Prelude.hpp"
#include <glm/mat4x4.hpp>

namespace engine::gl {

void AllocateAxesRenderer();

void RenderAxes(glm::mat4 const& mvp);

} // namespace engine::gl
