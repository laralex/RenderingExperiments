#pragma once

#include <cstdint>
#include <glad/gl.h>
#include <optional>
#include <string_view>

#include "engine/gl/Capabilities.hpp"
#include <engine/gl/Helpers.hpp>
#include "engine/gl/Extensions.hpp"
#include <engine/gl/Debug.hpp>

#ifdef XDEBUG
#define SPDLOG_COMPILED_LIB 1
#include "spdlog/spdlog.h"
// clang-format off
#define XLOG_LVL(lvl, format, ...) { spdlog::log(lvl, format, __VA_ARGS__); }
#define XLOG(format, ...) { spdlog::info(format, __VA_ARGS__); }
#define XLOGW(format, ...) { spdlog::warning(format, __VA_ARGS__); }
#define XLOGE(format, ...) { spdlog::error(format, __VA_ARGS__); }
// clang-format on
#else
#define XLOG_LVL(lvl, format, ...)
#define XLOG(format, ...)
#define XLOGW(format, ...)
#define XLOGE(format, ...)
#endif // XDEBUG

struct GLFWwindow;

namespace engine {

#ifdef XDEBUG
constexpr bool DEBUG_BUILD = true;
#else
constexpr bool DEBUG_BUILD = false;
#endif // XDEBUG

} // namespace engine