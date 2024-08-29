#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Shader.hpp"

#include <fstream>
#include <sstream>
#include <string_view>

namespace engine {

auto LoadTextFile [[nodiscard]] (std::string_view const filepath) -> std::string;

using FileSizeCallback = std::function<std::pair<uint8_t*, size_t>(size_t)>;

// Argument sizeCallback accepts the file length in bytes, and returns destination memory pointer + its length in bytes.
// This allows to react on the file size to return an appropriate destination storage
// Function returns the number of bytes actually written
auto LoadBinaryFile [[nodiscard]] (std::string_view const filepath, FileSizeCallback sizeCallback) -> size_t;

} // namespace engine

namespace engine::gl {

auto LoadShaderCode [[nodiscard]] (std::string_view const filepath, CpuView<ShaderDefine const> defines) -> std::string;
} // namespace engine::gl
