#pragma once

#include <cstdint>

#ifdef XDEBUG
    #define SPDLOG_COMPILED_LIB 1
    #include "spdlog/spdlog.h"
    #define XLOG(format, ...) { SPDLOG_INFO(format, __VA_ARGS__); }
#else
    #define XLOG(format, ...)
#endif // XDEBUG

namespace engine {

using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
using usize = std::size_t;

using i8 = int8_t;
using i32 = int32_t;
using i64 = int64_t;
using isize = std::ptrdiff_t;

using f32 = float;
using f64 = double;
} // namespace engine
