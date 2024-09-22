#pragma once


#ifdef XDEBUG

#define SPDLOG_COMPILED_LIB 1
#define SPDLOG_NO_EXCEPTIONS 1
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "spdlog/spdlog.h"
#pragma clang diagnostic pop
// clang-format off
#define X_VA_ARGS(...) , ##__VA_ARGS__
#define XLOG_LVL(lvl, format, ...) { spdlog::log(lvl, format X_VA_ARGS(__VA_ARGS__)); }
#define XLOG(format, ...) { spdlog::info(format X_VA_ARGS(__VA_ARGS__)); }
#define XLOGW(format, ...) { spdlog::warn(format X_VA_ARGS(__VA_ARGS__)); }
#define XLOGE(format, ...) { spdlog::error(format X_VA_ARGS(__VA_ARGS__)); }
// clang-format on
#else
#define XLOG_LVL(lvl, format, ...)
#define XLOG(format, ...)
#define XLOGW(format, ...)
#define XLOGE(format, ...)
#endif // XDEBUG