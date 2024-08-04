#pragma once

#include "engine/Prelude.hpp"

#include <fstream>
#include <sstream>
#include <string_view>

namespace engine {

auto LoadTextFile [[nodiscard]] (std::string_view const filepath) -> std::string;

} // namespace engine
