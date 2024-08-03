#pragma once

#include <string_view>
#include <cstdint>
#include <glad/gl.h>

#ifdef XDEBUG
#define SPDLOG_COMPILED_LIB 1
#include "spdlog/spdlog.h"
#define XLOG(format, ...)                                                                                              \
    { SPDLOG_INFO(format, __VA_ARGS__); }
#define XLOGE(format, ...)                                                                                             \
    { SPDLOG_ERROR(format, __VA_ARGS__); }
#else
#define XLOG(format, ...)
#define XLOGE(format, ...)
#endif // XDEBUG

struct GLFWwindow;

namespace engine {

using u8    = uint8_t;
using u32   = uint32_t;
using u64   = uint64_t;
using usize = std::size_t;

using i8    = int8_t;
using i32   = int32_t;
using i64   = int64_t;
using isize = std::ptrdiff_t;

using f32 = float;
using f64 = double;

struct vec2 {
    union {
        f32 x;
        f32 width;
    };
    union {
        f32 y;
        f32 height;
    };
};

struct ivec2 {
    union {
        i32 x;
        i32 width;
    };
    union {
        i32 y;
        i32 height;
    };
};

struct vec3 {
    union {
        f32 x;
        f32 width;
    };
    union {
        f32 y;
        f32 height;
    };
    union {
        f32 z;
        f32 depth;
    };
};

struct ivec3 {
    union {
        i32 x;
        i32 width;
    };
    union {
        i32 y;
        i32 height;
    };
    union {
        i32 z;
        i32 depth;
    };
};

struct vec4 {
    union {
        f32 x;
        f32 r;
    };
    union {
        f32 y;
        f32 g;
    };
    union {
        f32 z;
        f32 b;
    };
    union {
        f32 w;
        f32 a;
    };
};

void CheckOpenGLError(const char* stmt, const char* fname, int line, bool fatal);

#ifdef XDEBUG
#define GLCALL(stmt) \
    do { \
        stmt; \
        engine::CheckOpenGLError(#stmt, __FILE__, __LINE__, /* fatal */ false); \
    } while (0)
#else
#define GLCALL(stmt) stmt
#endif

} // namespace engine
