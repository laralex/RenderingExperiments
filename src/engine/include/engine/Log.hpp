#pragma once

#define SPDLOG_COMPILED_LIB 1
#define SPDLOG_NO_EXCEPTIONS 1

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <spdlog/spdlog.h>
#pragma clang diagnostic pop

// clang-format off
#define X_VA_ARGS(...) , ##__VA_ARGS__
#define XLOG_LVL(lvl, format, ...) { spdlog::log(lvl, format X_VA_ARGS(__VA_ARGS__)); }

#if 1
#   define XLOGE(format, ...) { spdlog::error(format X_VA_ARGS(__VA_ARGS__)); }
#endif

#if defined(XDEBUG)
#   define XLOG(format, ...) { spdlog::info(format X_VA_ARGS(__VA_ARGS__)); }
#endif

// TODO: leaving just "if defined(XDEBUG)" throws a ton of linking errors in release build...
#if 1 || defined(XDEBUG)
#   define XLOGW(format, ...) { spdlog::warn(format X_VA_ARGS(__VA_ARGS__)); }
#endif

#if defined(XDEBUG) && defined(XVERBOSE)
#   define XLOGD(format, ...) { spdlog::debug(format X_VA_ARGS(__VA_ARGS__)); }
#endif

// define macros if haven't yet

#ifndef XLOGE
#   define XLOGE(format, ...) {}
#endif

#ifndef XLOGW
#   define XLOGW(format, ...) {}
#endif

#ifndef XLOG
#   define XLOG(format, ...) {}
#endif

#ifndef XLOGD
#   define XLOGD(format, ...) {}
#endif

// clang-format on

namespace engine {

void InitLogging();

} // namespace engine
