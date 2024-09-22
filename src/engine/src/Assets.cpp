#include "engine/Assets.hpp"
#include "engine/gl/Texture.hpp"
#include "engine_private/Prelude.hpp"

#include <stb_image.h>

namespace engine {

ENGINE_EXPORT auto LoadTextFile(std::string_view const filepath) -> std::string {
    std::ostringstream ss;
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
    file.read(reinterpret_cast<char*>(destination.data), destination.NumElements());
    file.close();

    XLOG("Loaded binary file: {}", filepath);
    return fileLength;
}

ImageLoader::~ImageLoader() noexcept {
    for (auto decodedImage : loadedImages_) {
        stbi_image_free(static_cast<void*>(decodedImage.second.data));
    }
    loadedImages_.clear();
}

auto ImageLoader::ImageData(int32_t loadedImageId) const -> CpuView<uint8_t const> {
    auto find = loadedImages_.find(loadedImageId);
    if (find == loadedImages_.cend()) { return CpuView<uint8_t>{}; }
    return find->second;
}

auto ImageLoader::Load(std::string_view const filepath, int32_t numDesiredChannels) -> std::optional<LoadInfo> {
    auto numBytes = LoadBinaryFile(filepath, [&](size_t filesize) {
        temporaryBuffer_.resize(filesize);
        return CpuMemory{temporaryBuffer_.data(), filesize};
    });
    return Load(CpuMemory{temporaryBuffer_.data(), numBytes}, numDesiredChannels);
}

auto ImageLoader::Load(CpuMemory<uint8_t> encodedImageData, int32_t numDesiredChannels) -> std::optional<LoadInfo> {
    LoadInfo result{};
    if (int ok = stbi_info_from_memory(
            encodedImageData.data, encodedImageData.NumElements(), &result.width, &result.height,
            &result.numChannelsInFile);
        ok == 0) {
        latestError_ = stbi_failure_reason();
        return std::nullopt;
    }

    auto* decodedImageData = stbi_load_from_memory(
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
        stbi_image_free(static_cast<void*>(findRemoved->second.data));
        XLOGW("Unloaded CPU image data ID={}", findRemoved->first);
        loadedImages_.erase(findRemoved);
    }

    auto numDecodedBytes        = result.width * result.height * result.numChannelsDecoded * sizeof(uint8_t);
    loadedImages_[nextImageId_] = CpuView{decodedImageData, numDecodedBytes};
    result.numChannelsDecoded   = numDesiredChannels;
    result.numDecodedBytes      = numDecodedBytes;
    result.loadedImageId        = nextImageId_;
    ++nextImageId_;
    return std::optional{result};
}

} // namespace engine

namespace engine::gl {

auto LoadTexture [[nodiscard]] (GlContext const& gl, LoadTextureArgs const& args) -> std::optional<Texture> {
    auto cpuImageInfo = args.loader.Load(args.filepath, args.numChannels);
    if (!cpuImageInfo) {
        XLOGE("Failed to load texture: {}", args.loader.LatestError());
        return std::nullopt;
    }
    assert(cpuImageInfo);
    auto texture = gl::Texture::Allocate2D(
        gl, GL_TEXTURE_2D, glm::ivec3(cpuImageInfo->width, cpuImageInfo->height, 0), args.format, args.name);
    auto cpuImage     = args.loader.ImageData(cpuImageInfo->loadedImageId);
    auto textureGuard = gl::TextureCtx{texture}.Fill2D(gl::TextureCtx::FillArgs{
        .dataFormat = GL_RGB,
        .dataType   = GL_UNSIGNED_BYTE,
        .data       = cpuImage.data,
        .size       = texture.Size(),
        .mipLevel   = 0,
    });

    if (args.withMips) { (void)textureGuard.GenerateMipmaps(); }
    return texture;
}

} // namespace engine::gl

namespace engine::gl::shader {
auto LoadShaderCode(std::string_view const filepath, ShaderType type, CpuView<shader::Define const> defines)
    -> std::string {
    std::string code          = LoadTextFile(filepath);
    static bool isInitialized = false;
    static IncludeRegistry includeCommon{};
    static IncludeRegistry includeVertex{};
    static IncludeRegistry includeFragment{};
    if (!isInitialized) {
        LoadCommonIncludes(includeCommon);
        includeVertex.insert(includeCommon.begin(), includeCommon.end());
        LoadVertexIncludes(includeVertex);
        includeFragment.insert(includeCommon.begin(), includeCommon.end());
        LoadFragmentIncludes(includeFragment);
        isInitialized = true;
    };
    auto const& includeRegistry = type == ShaderType::VERTEX ? includeVertex : includeFragment;
    code                        = GenerateCode(code, includeRegistry, defines);
    return code;
}

} // namespace engine::gl::shader
