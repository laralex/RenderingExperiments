#pragma once

#include <cstdint>
#include <glad/gl.h>
#include <optional>
#include <string_view>

#include "engine/gl/Capabilities.hpp"
#include "engine/gl/Extensions.hpp"
#include <engine/gl/Common.hpp>
#include <engine/gl/Debug.hpp>

#ifdef XDEBUG
#define SPDLOG_COMPILED_LIB 1
#include "spdlog/spdlog.h"
// clang-format off
#define XLOG_LVL(lvl, format, ...) { spdlog::log(lvl, format, __VA_ARGS__); }
#define XLOG(format, ...) { spdlog::info(format, __VA_ARGS__); }
#define XLOGW(format, ...) { spdlog::warn(format, __VA_ARGS__); }
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

// aka simplified std::ranges::views::strided_view from C++23
// NOTE: currently only works with pointers to const (T = const Foo)
template <typename T> struct CpuView {
    uint8_t const* data;
    uint8_t const* dataEnd;
    size_t byteStride;

    explicit CpuView()
        : data(nullptr)
        , dataEnd(nullptr)
        , byteStride(0) { }
    explicit CpuView(T* data, size_t numElements, size_t byteStride = sizeof(T))
        : data(reinterpret_cast<uint8_t const*>(data))
        , dataEnd(reinterpret_cast<uint8_t const*>(data) + numElements * byteStride)
        , byteStride(byteStride) { }

    auto operator[](size_t idx) -> T* {
        auto* item = data + idx * byteStride;
        return (item < dataEnd) ? reinterpret_cast<T*>(item) : nullptr;
    }

    explicit operator bool() const { return data != nullptr; }
    auto NumElements() const -> size_t { return (dataEnd - data) / byteStride; }
};

} // namespace engine