#pragma once

#include "engine/Precompiled.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace engine::gl {

class Texture final {

public:
#define Self Texture
    explicit Self() noexcept = default;
    ~Self() noexcept { Dispose(); };
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    static auto Allocate2D [[nodiscard]] (
        GlContext const& gl, GLenum slotTarget, glm::ivec2 size, GLenum internalFormat, std::string_view name = {})
    -> Texture;
    // NOTE: sampleStencilOnly controls GL_DEPTH_STENCIL_TEXTURE_MODE parameter.
    // when true: stencil value is read in shader (depth value can't be retrieved)
    // when false: depth value is read in shader (stencil value can't be retrieved)
    static auto AllocateZS [[nodiscard]] (
        GlContext const& gl, glm::ivec2 size, GLenum internalFormat, bool sampleStencilOnly = false,
        std::string_view name = {}) -> Texture;

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

    friend class TextureCtx;
};

// Helper object, binds GL texture in ctor, unbinds it in dtor
// No two instances of the class should exist in one scope (checked by assert)
class TextureCtx final {
public:
#define Self TextureCtx
    explicit Self(Texture const& useTexture) noexcept;
    ~Self() noexcept;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    // NOTE: nodiscard is not needed
    auto GenerateMipmaps(GLint minLevel = 0, GLint maxLevel = 1000) & -> TextureCtx&;
    auto GenerateMipmaps [[nodiscard]] (GLint minLevel = 0, GLint maxLevel = 1000) && -> TextureCtx&&;

    struct FillArgs {
        GLenum dataFormat   = GL_NONE;
        GLenum dataType     = GL_NONE;
        uint8_t const* data = nullptr;
        glm::ivec3 size     = glm::ivec3{0};
        GLint mipLevel      = 0;
    };
    auto Fill2D(FillArgs const& args) & -> TextureCtx&;
    auto Fill2D [[nodiscard]] (FillArgs const& args) && -> TextureCtx&&;

private:
    static GlHandle contextTexture_;
    static GLenum contextTarget_;
    static bool hasInstances_;
};

} // namespace engine::gl