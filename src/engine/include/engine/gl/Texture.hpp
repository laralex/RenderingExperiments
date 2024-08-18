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
        [[nodiscard]] (GLenum slotTarget, glm::ivec2 size, GLenum internalFormat, std::string_view name = {})
        -> Texture;
    // NOTE: sampleStencilOnly controls GL_DEPTH_STENCIL_TEXTURE_MODE parameter.
    // when true: stencil value is read in shader (depth value can't be retrieved)
    // when false: depth value is read in shader (stencil value can't be retrieved)
    static auto AllocateZS [[nodiscard]] (
        glm::ivec2 size, GLenum internalFormat, bool sampleStencilOnly = false, std::string_view name = {}) -> Texture;

    auto Id [[nodiscard]] () const -> GLuint { return textureId_; }
    auto Size [[nodiscard]] () const -> glm::ivec3 { return size_; }
    auto TextureSlotTarget [[nodiscard]] () const -> GLenum { return target_; }
    auto Is1D() const -> bool { return target_ == GL_TEXTURE_1D | target_ == GL_TEXTURE_1D_ARRAY; }
    auto Is2D() const -> bool {
        return target_ == GL_TEXTURE_2D | target_ == GL_TEXTURE_2D_ARRAY | target_ == GL_TEXTURE_2D_MULTISAMPLE
            | target_ == GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
    }
    auto Is3D() const -> bool { return target_ == GL_TEXTURE_3D; }
    auto IsTextureArray() const -> bool {
        return target_ == GL_TEXTURE_1D_ARRAY | target_ == GL_TEXTURE_2D_ARRAY
            | target_ == GL_TEXTURE_2D_MULTISAMPLE_ARRAY | target_ == GL_TEXTURE_CUBE_MAP_ARRAY;
    }
    auto IsCubemap() const -> bool { return target_ == GL_TEXTURE_CUBE_MAP | target_ == GL_TEXTURE_CUBE_MAP_ARRAY; }

private:
    void Dispose();

    GlHandle textureId_{GL_NONE};
    GLenum target_{GL_NONE};
    GLenum internalFormat_{GL_NONE};
    glm::ivec3 size_{};
};

// Helper object, binds GL texture in ctor, unbinds it in dtor
// No two instances of the class should exist in one scope (checked by assert)
class TextureCtx final {
public:
#define Self TextureCtx
    explicit Self(Texture const& useTexture);
    ~Self();
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    auto GenerateMipmaps [[nodiscard]] (GLint minLevel = 0, GLint maxLevel = 1000) -> TextureCtx&&;
    auto Fill2D
        [[nodiscard]] (GLenum dataFormat, GLenum dataType, uint8_t const* data, glm::ivec3 size, GLint miplevel = 0)
        -> TextureCtx&&;

private:
    static GlHandle contextTexture_;
    static GLenum contextTarget_;
    static bool hasInstances_;
};

} // namespace engine::gl