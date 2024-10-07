#include "engine/gl/Texture.hpp"
#include "engine/gl/Context.hpp"

#include "engine_private/Prelude.hpp"

namespace {

static void GenerateMipmapsImpl(GLenum target, GLuint texture, GLint minLevel, GLint maxLevel) {
    GLCALL(glBindTexture(target, texture));
    GLCALL(glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, minLevel));
    GLCALL(glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, maxLevel));
    GLCALL(glGenerateMipmap(target));
}

static void Fill2DImpl(GLenum target, engine::gl::TextureCtx::FillArgs const& args) {
    GLint offsetX = 0, offsetY = 0;
    GLCALL(glTexSubImage2D(
        target, args.mipLevel, offsetX, offsetY, args.size.x, args.size.y, args.dataFormat, args.dataType, args.data));
}

} // namespace

namespace engine::gl {

ENGINE_STATIC bool TextureCtx::hasInstances_{false};
ENGINE_STATIC GlHandle TextureCtx::contextTexture_{GL_NONE};
ENGINE_STATIC GLenum TextureCtx::contextTarget_{GL_NONE};

ENGINE_EXPORT TextureCtx::TextureCtx(Texture const& useTexture) noexcept {
    assert(!hasInstances_);
    contextTexture_.UnsafeAssign(useTexture.textureId_);
    contextTarget_ = useTexture.TextureSlotTarget();
    GLCALL(glBindTexture(contextTarget_, contextTexture_));
    hasInstances_ = true;
}

ENGINE_EXPORT TextureCtx::~TextureCtx() noexcept {
    if (!hasInstances_) { return; }
    // assert(hasInstances_);
    contextTexture_.UnsafeReset();
    GLCALL(glBindTexture(contextTarget_, contextTexture_));
    hasInstances_ = false;
}

ENGINE_EXPORT void Texture::Dispose() {
    if (textureId_ == GL_NONE) { return; }
    // LogDebugLabel(*this, "Texture object was disposed");
    XLOG("Texture object was disposed: 0x{:08X}", GLuint(textureId_));
    GLCALL(glDeleteTextures(1, textureId_.Ptr()));
    textureId_.UnsafeReset();
}

ENGINE_EXPORT auto Texture::Allocate2D(
    GlContext& gl, GLenum textureType, glm::ivec2 size, GLenum internalFormat, std::string_view name) -> Texture {
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
    GLCALL(glGenTextures(1, texture.textureId_.Ptr()));
    texture.target_         = textureType;
    texture.size_           = glm::ivec3(size.x, size.y, 0);
    texture.internalFormat_ = internalFormat;

    GLCALL(glBindTexture(texture.target_, texture.textureId_));

    if (!gl.Extensions().Supports(GlExtensions::ARB_texture_storage)) {
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
        GLCALL(glTexStorage2D(texture.target_, numLevels, texture.internalFormat_, texture.size_.x, texture.size_.y));
    }
    GLCALL(glTexParameteri(texture.target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(texture.target_, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(texture.target_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCALL(glTexParameteri(texture.target_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCALL(glTexParameteri(texture.target_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    if (!name.empty()) {
        DebugLabel(gl, texture, name);
        LogDebugLabel(gl, texture, "Texture was allocated");
    }
    return texture;
}

ENGINE_EXPORT auto Texture::AllocateZS(
    GlContext& gl, glm::ivec2 size, GLenum internalFormat, bool sampleStencilOnly, std::string_view name)
    -> Texture {
    {
        GLenum f = internalFormat;
        assert(
            f == GL_DEPTH_COMPONENT || f == GL_DEPTH_COMPONENT16 || f == GL_DEPTH_COMPONENT24
            || f == GL_DEPTH_COMPONENT32F);
    }

    Texture texture = Allocate2D(gl, GL_TEXTURE_2D, size, internalFormat, name);
    glTexParameteri(
        texture.target_, GL_DEPTH_STENCIL_TEXTURE_MODE, sampleStencilOnly ? GL_STENCIL_INDEX : GL_DEPTH_COMPONENT);
    return texture;
}

ENGINE_EXPORT auto TextureCtx::GenerateMipmaps(GLint minLevel, GLint maxLevel) & -> TextureCtx& {
    GenerateMipmapsImpl(contextTarget_, contextTexture_, minLevel, maxLevel);
    return *this;
}

ENGINE_EXPORT auto TextureCtx::GenerateMipmaps(GLint minLevel, GLint maxLevel) && -> TextureCtx&& {
    GenerateMipmapsImpl(contextTarget_, contextTexture_, minLevel, maxLevel);
    return std::move(*this);
}

ENGINE_EXPORT auto TextureCtx::Fill2D(TextureCtx::FillArgs const& args) & -> TextureCtx& {
    Fill2DImpl(contextTarget_, args);
    return *this;
}

ENGINE_EXPORT auto TextureCtx::Fill2D(TextureCtx::FillArgs const& args) && -> TextureCtx&& {
    Fill2DImpl(contextTarget_, args);
    return std::move(*this);
}

} // namespace engine::gl