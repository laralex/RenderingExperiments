#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Shader.hpp"

#include <fstream>
#include <sstream>
#include <string_view>

namespace engine {

auto LoadTextFile [[nodiscard]] (std::string_view const filepath) -> std::string;

} // namespace engine

namespace engine::gl {

auto LoadShaderCode
    [[nodiscard]] (std::string_view const filepath, ShaderDefine const* defines, int32_t limit, int32_t stride = 1)
    -> std::string;
} // namespace engine::gl
