#include "engine/GlTexture.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

void Texture::Dispose() {
    if (textureId_ == GL_NONE) { return; }
    LogDebugLabel(*this, "Texture object was disposed");
    GLCALL(glDeleteTextures(1, &textureId_.id));
    textureId_.id = GL_NONE;
}

auto Texture::Allocate2D(GLenum textureType, glm::ivec2 size, GLenum internalFormat, std::string_view name) -> Texture {
    Texture out{};
    GLCALL(glGenTextures(1, &out.textureId_.id));
    out.textureType_ = textureType;
    out.size_ = glm::ivec3(size.x, size.y, 0);
    out.internalFormat_ = internalFormat;
    GLCALL(glBindTexture(out.textureType_, out.textureId_.id));
    constexpr GLint border = 0;
    GLCALL(glTexImage2D(out.textureType_, 0, out.internalFormat_, out.size_.x, out.size_.y, border, GL_RGB, GL_UNSIGNED_BYTE, nullptr));
    if (!name.empty()) {
        DebugLabel(out, name);
        LogDebugLabel(out, "Texture was allocated");
    }
    return out;
}

void Texture::GenerateMipmaps() {
    GLCALL(glBindTexture(textureType_, textureId_.id));
    GLCALL(glGenerateMipmap(textureType_));
}

void Texture::Fill2D(GLenum dataFormat, GLenum dataType, uint8_t const* data, GLint miplevel) {
    GLint offsetX = 0, offsetY = 0;
    GLCALL(glTexSubImage2D(textureType_, miplevel, offsetX, offsetY, size_.x, size_.y, dataFormat, dataType, data));
}

} // namespace engine::gl