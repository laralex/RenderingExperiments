#pragma once

#include "engine/Prelude.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace engine::gl {

class Texture final {

public:
#define Self Texture
    explicit Self() = default;
    ~Self() { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate2D
        [[nodiscard]] (GLenum textureType, glm::ivec2 size, GLenum internalFormat, std::string_view name = {})
        -> Texture;
    // NOTE: sampleStencilOnly controls GL_DEPTH_STENCIL_TEXTURE_MODE parameter.
    // when true: stencil value is read in shader (depth value can't be retrieved)
    // when false: depth value is read in shader (stencil value can't be retrieved)
    static auto AllocateZS [[nodiscard]] (
        glm::ivec2 size, GLenum internalFormat, bool sampleStencilOnly = false, std::string_view name = {}) -> Texture;
    void GenerateMipmaps(GLint minLevel = 0, GLint maxLevel = 1000);
    void Fill2D(GLenum dataFormat, GLenum dataType, uint8_t const* data, GLint miplevel = 0);
    auto Id [[nodiscard]] () const -> GLuint { return textureId_; }

private:
    void Dispose();

    GlHandle textureId_{GL_NONE};
    GLenum textureType_{GL_NONE};
    GLenum internalFormat_{GL_NONE};
    glm::ivec3 size_{};
};

} // namespace engine::gl