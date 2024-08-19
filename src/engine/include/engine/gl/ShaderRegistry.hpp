#pragma once

#include "engine/Prelude.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Shader.hpp"

#include <unordered_map>

namespace engine::gl {

enum class VertexShaderId : std::size_t {
    AXES = 0,
    BOX,
    FRUSTUM,
    TRIANGLE_FULLSCREEN,
    NUM_VERTEX_SHADERS,
};

enum class FragmentShaderId : std::size_t {
    AXES = 0,
    BLIT,
    CONSTANT,
    NUM_FRAGMENT_SHADERS,
};

class ShaderRegistry final {
public:
#define Self ShaderRegistry
    explicit Self();
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = default;
#undef Self

    // void Initialize();
    // auto IsInitialized [[nodiscard]] () { return isInitialized_; }

    std::unordered_map<VertexShaderId, std::string> vertexSrcCode_;
    std::unordered_map<FragmentShaderId, std::string> fragmentSrcCode_;
    std::unordered_map<VertexShaderId, GLuint> vertexShaders_;
    std::unordered_map<FragmentShaderId, GLuint> fragmentShaders_;
    // bool isInitialized_;
};

} // namespace engine::gl
