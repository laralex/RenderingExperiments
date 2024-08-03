#pragma once

#include "engine/Prelude.hpp"

#include <fstream>
#include <string_view>
#include <sstream>

namespace engine {

auto LoadTextFile[[nodiscard]](std::string_view const filepath) -> std::string;

} // namespace engine
