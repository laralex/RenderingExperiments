#pragma once

#include <cstdint>
#include <functional>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <cr.h>
#pragma clang diagnostic pop
#define ENGINE_STATIC CR_STATE

#include <concurrentqueue.h>

// #include "engine/CommonInterfaces.hpp"
#include "engine/gl/Common.hpp"

#include "engine/Log.hpp"
#include "engine/gl/GlCapabilities.hpp"
#include "engine/gl/Context.hpp"
#include "engine/gl/Debug.hpp"
#include "engine/gl/GlExtensions.hpp"
#include "engine/gl/GpuProgramRegistry.hpp"
#include "engine/gl/IGlDisposable.hpp"

struct GLFWwindow;

namespace engine {

#ifdef XDEBUG
constexpr bool XDEBUG_BUILD = true;
#else
constexpr bool XDEBUG_BUILD = false;
#endif // XDEBUG

#ifdef XVERBOSE
constexpr int32_t XVERBOSITY = 100;
#else
constexpr int32_t XVERBOSITY = 4;
#endif // XVERBOSE

template <typename T> constexpr uint32_t Bits(T value) { return static_cast<uint32_t>(value); }

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