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

    static auto Allocate2D [[nodiscard]] (GLenum textureType, glm::ivec2 size, GLenum internalFormat, std::string_view name = {}) -> Texture;
    void GenerateMipmaps();
    void Fill2D(GLenum dataFormat, GLenum dataType, uint8_t const* data, GLint miplevel = 0);
    auto Id [[nodiscard]] () const -> GLuint { return textureId_; }

private:
    void Dispose();

    GlHandle textureId_{GL_NONE};
    GLenum textureType_{GL_NONE};
    GLenum internalFormat_{GL_NONE};
    glm::ivec3 size_{};
};

}