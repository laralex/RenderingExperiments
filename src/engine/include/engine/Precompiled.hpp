#pragma once

#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <string_view>
#include <functional>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <cr.h>
#pragma clang diagnostic pop
#define ENGINE_STATIC CR_STATE

// #include "engine/CommonInterfaces.hpp"
#include "engine/gl/Common.hpp"

#include "engine/gl/Capabilities.hpp"
#include "engine/gl/Context.hpp"
#include "engine/gl/Debug.hpp"
#include "engine/gl/Extensions.hpp"
#include "engine/Log.hpp"

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
    MISSING_COLOR,
    NUM_COLORS // keep last !
};

constexpr glm::vec3 COLOR_PALETTE[static_cast<int32_t>(ColorCode::NUM_COLORS)]{
    {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},          {0.0f, 0.0f, 1.0f}, {1.0f, 0.5f, 0.0f},
    {1.0f, 1.0f, 0.0f}, {0.0f, 0.5f, 1.0f}, {0.5f, 0.0f, 1.0f},          {1.0f, 0.0f, 1.0f}, {0.2f, 0.1f, 0.0f},
    {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.4242f, 0.0042f, 0.4242f},
};

constexpr glm::vec3 COLOR_DEFAULT = COLOR_PALETTE[static_cast<int32_t>(ColorCode::MISSING_COLOR)];

constexpr glm::vec3 VEC_UP{0.0f, 0.0f, 1.0f};
constexpr glm::vec3 VEC_RIGHT{1.0f, 0.0f, 0.0f};
constexpr glm::vec3 VEC_FORWARD{0.0f, -1.0f, 0.0f};
constexpr glm::vec3 VEC_ONES{1.0f, 1.0f, 1.0f};

// aka simplified std::ranges::views::strided_view from C++23
// NOTE: currently only works with pointers to const (T = const Foo)
template <typename T> struct CpuView {
    using CpuViewConst = CpuView<std::add_const_t<T>>;
    using BytePtr      = std::conditional_t<std::is_const_v<T>, uint8_t const*, uint8_t*>;
    using VoidPtr      = std::conditional_t<std::is_const_v<T>, void const*, void*>;

    BytePtr data;
    BytePtr dataEnd;
    size_t byteStride;

    explicit CpuView()
        : data(nullptr)
        , dataEnd(nullptr)
        , byteStride(0) { }

    explicit CpuView(T* data, size_t numElements, ptrdiff_t byteOffset = 0, size_t byteStride = sizeof(T))
        requires(!std::is_same_v<T, void const> && !std::is_same_v<T, void>)
        : CpuView(reinterpret_cast<VoidPtr>(data), numElements, byteOffset, byteStride) { }

    explicit CpuView(VoidPtr data, size_t numElements, ptrdiff_t byteOffset = 0, size_t byteStride = sizeof(T))
        : data(reinterpret_cast<BytePtr>(data) + byteOffset)
        , dataEnd(this->data + numElements * byteStride)
        , byteStride(byteStride) { }

    auto operator[](size_t idx) -> T* {
        auto* item = data + idx * byteStride;
        return (item < dataEnd) ? reinterpret_cast<T*>(item) : nullptr;
    }

    explicit operator bool() const { return data != nullptr; }
    operator CpuViewConst const&() const { return reinterpret_cast<CpuViewConst const&>(*this); }
    auto Begin() const -> T* { return reinterpret_cast<T*>(data); }
    auto End() const -> T* { return reinterpret_cast<T*>(dataEnd); }
    auto NumElements() const -> size_t { return (dataEnd - data) / byteStride; }
    auto NumBytes() const -> size_t { return dataEnd - data; }
    auto IsContiguous() const -> bool { return byteStride == sizeof(T); }
    auto IsEmpty() const -> bool { return data == nullptr | std::distance(data, dataEnd) == 0; }
};

// CpuMemory is same as CpuView, but contiguous (i.e. byteStride is equal to datatype)
template <typename T> struct CpuMemory : CpuView<T> {
    explicit CpuMemory(T* data, size_t numElements, ptrdiff_t byteOffset = 0)
        : CpuView<T>(data, numElements, byteOffset, sizeof(T)) { }
    explicit CpuMemory()
        : CpuView<T>() { }
};

template <> struct CpuMemory<void> : CpuView<void> {
    explicit CpuMemory(void* data, size_t numElements, ptrdiff_t byteOffset = 0)
        : CpuView<void>(data, numElements, byteOffset, 1) { }
    explicit CpuMemory()
        : CpuView<void>() { }
};

template <> struct CpuMemory<void const> : CpuView<void const> {
    explicit CpuMemory(void const* data, size_t numElements, ptrdiff_t byteOffset = 0)
        : CpuView<void const>(data, numElements, byteOffset, 1) { }
    explicit CpuMemory()
        : CpuView<void const>() { }
};

struct StringEqual {
    using is_transparent = std::true_type;

    bool operator()(std::string_view l, std::string_view r) const noexcept { return l == r; }
};

struct StringHash {
    using hash_type      = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator() [[nodiscard]] (char const* str) const { return hash_type{}(str); }
    std::size_t operator() [[nodiscard]] (std::string_view str) const { return hash_type{}(str); }
    std::size_t operator() [[nodiscard]] (std::string const& str) const { return hash_type{}(str); }
};

template <typename IntT> void InvertTriangleWinding(std::vector<IntT>& triangleIndices) {
    int32_t const numIndices = std::size(triangleIndices);
    for (int32_t t = 0; t < numIndices; t += 3) {
        std::swap(triangleIndices[t + 1], triangleIndices[t + 2]);
    }
}

void InvertTriangleStripWinding(std::vector<uint16_t>& triangleIndices);

void InvertTriangleNormals(CpuView<glm::vec3> vertexData);

auto RotateByQuaternion [[nodiscard]] (glm::vec3 v, glm::quat q) -> glm::vec3;

} // namespace engine