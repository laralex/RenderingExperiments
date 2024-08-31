#include "engine/Assets.hpp"
#include "engine/gl/Texture.hpp"
#include "engine_private/Prelude.hpp"

#include <stb_image.h>

namespace engine {

ENGINE_EXPORT auto LoadTextFile(std::string_view const filepath) -> std::string {
    std::stringstream ss;
    std::ifstream file(filepath.data());
    if (!file.is_open()) {
        XLOGE("Failed to load text file: {}", filepath);
        return "";
    }
    ss << file.rdbuf();
    file.close();
    XLOG("Loaded text file: {}", filepath);
    return ss.str();
}

ENGINE_EXPORT auto LoadBinaryFile(std::string_view const filepath, FileSizeCallback sizeCallback) -> size_t {
    std::ifstream file(filepath.data(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        XLOGE("Failed to load binary file: {}", filepath);
        return 0;
    }

    size_t fileLength = file.tellg();
    if (fileLength == 0) {
        XLOGE("Failed to load binary file, it's empty: {}", filepath);
        return 0;
    }
    auto destination = sizeCallback(fileLength);

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(destination.first), destination.second);
    file.close();

    XLOG("Loaded binary file: {}", filepath);
    return fileLength;
}

ImageLoader::~ImageLoader() noexcept {
    for (auto decodedImage : loadedImages_) {
        stbi_image_free(static_cast<void*>(decodedImage.second));
    }
    loadedImages_.clear();
}

auto ImageLoader::ImageData(int32_t loadedImageId) const -> uint8_t const* {
    auto find = loadedImages_.find(loadedImageId);
    if (find == loadedImages_.cend()) { return nullptr; }
    return find->second;
}

auto ImageLoader::Load(std::string_view const filepath, int32_t numDesiredChannels) -> std::optional<LoadInfo> {
    auto numEncodedBytes = LoadBinaryFile(filepath, [&](size_t filesize) {
        temporaryBuffer_.resize(filesize);
        return std::pair{temporaryBuffer_.data(), filesize};
    });
    return Load(CpuView{temporaryBuffer_.data(), numEncodedBytes}, numDesiredChannels);
}

auto ImageLoader::Load(CpuView<uint8_t> encodedImageData, int32_t numDesiredChannels) -> std::optional<LoadInfo> {
    assert(encodedImageData.byteStride = sizeof(uint8_t));

    LoadInfo result{};
    if (int ok = stbi_info_from_memory(
            encodedImageData.data, encodedImageData.NumElements(), &result.width, &result.height,
            &result.numChannelsInFile);
        ok == 0) {
        latestError_ = stbi_failure_reason();
        return std::nullopt;
    }

    auto decodedImageData = stbi_load_from_memory(
        encodedImageData.data, encodedImageData.NumElements(), &result.width, &result.height, &result.numChannelsInFile,
        numDesiredChannels);
    if (!decodedImageData) {
        latestError_ = stbi_failure_reason();
        return std::nullopt;
    }

    // limit number of alive images (free the oldest one)
    // TODO: that's a very crude system, maybe store IDs in array
    auto findRemoved = loadedImages_.find(nextImageId_ - MAX_LOADED_IMAGES);
    if (findRemoved != loadedImages_.cend()) {
        stbi_image_free(static_cast<void*>(findRemoved->second));
        XLOGW("Unloaded CPU image data ID={}", findRemoved->first);
        loadedImages_.erase(findRemoved);
    }

    loadedImages_[nextImageId_] = decodedImageData;
    result.numChannelsDecoded   = numDesiredChannels;
    result.numDecodedBytes      = result.width * result.height * result.numChannelsDecoded * sizeof(uint8_t);
    result.loadedImageId        = nextImageId_;
    ++nextImageId_;
    return std::optional{result};
}

} // namespace engine

namespace engine::gl {

auto LoadShaderCode(std::string_view const filepath, CpuView<ShaderDefine const> defines) -> std::string {
    std::string code = LoadTextFile(filepath);
    if (defines) { code = AddShaderDefines(code, defines); }
    return code;
}

auto LoadTexture [[nodiscard]] (LoadTextureArgs const& args) -> std::optional<Texture> {
    auto cpuImage = args.loader.Load(args.filepath, args.numChannels);
    if (!cpuImage) {
        XLOGE("Failed to load texture: {}", args.loader.LatestError());
        return std::nullopt;
    }
    assert(cpuImage);
    auto texture = gl::Texture::Allocate2D(
        GL_TEXTURE_2D, glm::ivec3(cpuImage->width, cpuImage->height, 0), args.format, args.name);
    auto textureGuard = gl::TextureCtx{texture}.Fill2D(gl::TextureCtx::FillArgs{
        .dataFormat = GL_RGB,
        .dataType   = GL_UNSIGNED_BYTE,
        .data       = args.loader.ImageData(cpuImage->loadedImageId),
        .size       = texture.Size(),
        .mipLevel   = 0,
    });

    if (args.withMips) { (void)textureGuard.GenerateMipmaps(); }
    return texture;
}

} // namespace engine::gl
