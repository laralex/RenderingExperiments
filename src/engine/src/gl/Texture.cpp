#include "engine/gl/Texture.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

bool TextureCtx::hasInstances_{false};
GlHandle TextureCtx::contextTexture_{GL_NONE};
GLenum TextureCtx::contextTarget_{GL_NONE};

TextureCtx::TextureCtx(Texture const& useTexture) {
    assert(!hasInstances_);
    contextTexture_.id  = useTexture.Id();
    contextTarget_ = useTexture.TextureSlotTarget();
    GLCALL(glBindTexture(contextTarget_, contextTexture_.id));
    hasInstances_ = true;
}

TextureCtx::~TextureCtx() {
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    contextTexture_.id = GL_NONE;
    GLCALL(glBindTexture(contextTarget_, contextTexture_.id));
    hasInstances_ = false;
}

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
    texture.target_    = textureType;
    texture.size_           = glm::ivec3(size.x, size.y, 0);
    texture.internalFormat_ = internalFormat;

    GLCALL(glBindTexture(texture.target_, texture.textureId_.id));

    assert(GlExtensions::IsInitialized());
    if (!GlExtensions::Supports(GlExtensions::ARB_texture_storage)) {
        constexpr GLint border = 0;
        GLenum clientFormat    = GL_RGBA;
        GLenum clientType      = GL_UNSIGNED_BYTE;
        if (texture.internalFormat_ == GL_DEPTH_STENCIL | texture.internalFormat_ == GL_DEPTH24_STENCIL8
            | texture.internalFormat_ == GL_DEPTH32F_STENCIL8) {
            clientFormat = GL_DEPTH_STENCIL;
            clientType   = GL_UNSIGNED_INT_24_8;
        } else if (
            texture.internalFormat_ == GL_DEPTH_COMPONENT | texture.internalFormat_ == GL_DEPTH_COMPONENT16
            | texture.internalFormat_ == GL_DEPTH_COMPONENT24 | texture.internalFormat_ == GL_DEPTH_COMPONENT32
            | texture.internalFormat_ == GL_DEPTH_COMPONENT32F) {
            clientFormat = GL_DEPTH_COMPONENT;
            clientType   = GL_UNSIGNED_INT;
        };
        GLCALL(glTexImage2D(
            texture.target_, 0, texture.internalFormat_, texture.size_.x, texture.size_.y, border, clientFormat,
            clientType, nullptr));
    } else {
        // immutable texture (storage requirements can't change, but faster runtime check of texture completeness)
        constexpr GLsizei numLevels = 1;
        GLCALL(glTexStorage2D(
            texture.target_, numLevels, texture.internalFormat_, texture.size_.x, texture.size_.y));
    }
    GLCALL(glTexParameteri(texture.target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(texture.target_, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(texture.target_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCALL(glTexParameteri(texture.target_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCALL(glTexParameteri(texture.target_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

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
        texture.target_, GL_DEPTH_STENCIL_TEXTURE_MODE, sampleStencilOnly ? GL_STENCIL_INDEX : GL_DEPTH_COMPONENT);
    return texture;
}

auto TextureCtx::GenerateMipmaps(GLint minLevel, GLint maxLevel) -> TextureCtx&& {
    GLCALL(glBindTexture(contextTarget_, contextTexture_.id));
    GLCALL(glTexParameteri(contextTarget_, GL_TEXTURE_BASE_LEVEL, minLevel));
    GLCALL(glTexParameteri(contextTarget_, GL_TEXTURE_MAX_LEVEL, maxLevel));
    GLCALL(glGenerateMipmap(contextTarget_));
    return std::move(*this);
}

auto TextureCtx::Fill2D(GLenum dataFormat, GLenum dataType, uint8_t const* data, glm::ivec3 size, GLint miplevel)
    -> TextureCtx&& {
    GLint offsetX = 0, offsetY = 0;
    GLCALL(
        glTexSubImage2D(contextTarget_, miplevel, offsetX, offsetY, size.x, size.y, dataFormat, dataType, data));
    return std::move(*this);
}

} // namespace engine::gl