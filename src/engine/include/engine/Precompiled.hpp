#pragma once

#include <cstdint>
#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <string_view>

#include "engine/gl/Capabilities.hpp"
#include "engine/gl/Common.hpp"
#include "engine/gl/Debug.hpp"
#include "engine/gl/Extensions.hpp"

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

enum class ColorCode : int32_t {
    RED = 0,
    GREEN,
    BLUE,
    ORANGE,
    YELLOW,
    CYAN,
    PURPLE,
    PINK,
    BROWN,
    WHITE,
    GRAY,
    BLACK,
    NUM_COLORS
};

constexpr glm::vec3 COLOR_PALETTE[]{
    {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
    {1.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.5f, 1.0f}, {0.5f, 0.0f, 1.0f},
    {1.0f, 0.0f, 1.0f}, {0.2f, 0.1f, 0.0f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f},
};

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

struct string_equal {
    using is_transparent = std::true_type;

    bool operator()(std::string_view l, std::string_view r) const noexcept { return l == r; }
};

struct string_hash {
    using hash_type      = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator()(const char* str) const { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
    std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
};

template <typename IntT> void InvertTriangleWinding(std::vector<IntT>& triangleIndices) {
    int32_t const numIndices = std::size(triangleIndices);
    for (int32_t t = 0; t < numIndices; t += 3) {
        std::swap(triangleIndices[t + 1], triangleIndices[t + 2]);
    }
}

void InvertTriangleStripWinding(std::vector<uint16_t>& triangleIndices);

void InvertTriangleNormals(void* vertexData, int32_t normalsByteOffset, int32_t vertexStride, int32_t numVertices);

} // namespace engine