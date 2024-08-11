#include "engine/gl/Texture.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

void Texture::Dispose() {
    if (textureId_ == GL_NONE) { return; }
    LogDebugLabel(*this, "Texture object was disposed");
    GLCALL(glDeleteTextures(1, &textureId_.id));
    textureId_.id = GL_NONE;
}

auto Texture::Allocate2D(GLenum textureType, glm::ivec2 size, GLenum internalFormat, std::string_view name) -> Texture {
    {
        GLenum t = textureType;
        assert(
            t == GL_TEXTURE_2D || t == GL_PROXY_TEXTURE_2D || t == GL_TEXTURE_1D_ARRAY || t == GL_PROXY_TEXTURE_1D_ARRAY
            || t == GL_TEXTURE_RECTANGLE || t == GL_PROXY_TEXTURE_RECTANGLE || t == GL_TEXTURE_CUBE_MAP_POSITIVE_X
            || t == GL_TEXTURE_CUBE_MAP_NEGATIVE_X || t == GL_TEXTURE_CUBE_MAP_POSITIVE_Y
            || t == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y || t == GL_TEXTURE_CUBE_MAP_POSITIVE_Z
            || t == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z || t == GL_PROXY_TEXTURE_CUBE_MAP);
    }

    Texture texture{};
    GLCALL(glGenTextures(1, &texture.textureId_.id));
    texture.textureType_    = textureType;
    texture.size_           = glm::ivec3(size.x, size.y, 0);
    texture.internalFormat_ = internalFormat;
    GLCALL(glBindTexture(texture.textureType_, texture.textureId_.id));
    constexpr GLint border = 0;
    GLCALL(glTexImage2D(
        texture.textureType_, 0, texture.internalFormat_, texture.size_.x, texture.size_.y, border, GL_RGB,
        GL_UNSIGNED_BYTE, nullptr));
    if (!name.empty()) {
        DebugLabel(texture, name);
        LogDebugLabel(texture, "Texture was allocated");
    }
    return texture;
}

auto Texture::AllocateZS(glm::ivec2 size, GLenum internalFormat, bool sampleStencilOnly, std::string_view name)
    -> Texture {
    {
        GLenum f = internalFormat;
        assert(
            f == GL_DEPTH_COMPONENT || f == GL_DEPTH_COMPONENT16 || f == GL_DEPTH_COMPONENT24
            || f == GL_DEPTH_COMPONENT32F);
    }

    Texture texture = Allocate2D(GL_TEXTURE_2D, size, internalFormat, name);
    glTexParameteri(
        texture.textureType_, GL_DEPTH_STENCIL_TEXTURE_MODE, sampleStencilOnly ? GL_STENCIL_INDEX : GL_DEPTH_COMPONENT);
    return texture;
}

void Texture::GenerateMipmaps(GLint minLevel, GLint maxLevel) {
    GLCALL(glBindTexture(textureType_, textureId_.id));
    GLCALL(glTexParameteri(textureType_, GL_TEXTURE_BASE_LEVEL, minLevel));
    GLCALL(glTexParameteri(textureType_, GL_TEXTURE_MAX_LEVEL, maxLevel));
    GLCALL(glGenerateMipmap(textureType_));
}

void Texture::Fill2D(GLenum dataFormat, GLenum dataType, uint8_t const* data, GLint miplevel) {
    GLint offsetX = 0, offsetY = 0;
    GLCALL(glTexSubImage2D(textureType_, miplevel, offsetX, offsetY, size_.x, size_.y, dataFormat, dataType, data));
}

} // namespace engine::gl