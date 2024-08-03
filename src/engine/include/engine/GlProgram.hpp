#pragma once

#include "engine/Prelude.hpp"

namespace engine {

auto CompileShader [[nodiscard]] (GLenum shaderType, std::string_view code) -> GLuint;

auto CompileGraphicsProgram [[nodiscard]] (GLuint vertexShader, GLuint fragmentShader) -> GLuint;

} // namespace engine
