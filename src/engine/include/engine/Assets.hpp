#pragma once

#include "engine/Precompiled.hpp"
#include "engine/gl/Shader.hpp"

#include <fstream>
#include <sstream>
#include <string_view>

namespace engine {

auto LoadTextFile [[nodiscard]] (std::string_view const filepath) -> std::string;

using FileSizeCallback = std::function<CpuMemory<uint8_t>(size_t)>;

// Argument sizeCallback accepts the file length in bytes, and returns destination memory pointer + its length in bytes.
// This allows to react on the file size to return an appropriate destination storage
// Function returns the number of bytes actually written
auto LoadBinaryFile [[nodiscard]] (std::string_view const filepath, FileSizeCallback sizeCallback) -> size_t;

struct ImageLoader final {

public:
#define Self ImageLoader
    explicit Self(size_t temporaryBufferNumBytes = 4 * 1024 * 1024) noexcept
        : temporaryBuffer_(temporaryBufferNumBytes)
        , loadedImages_{}
        , nextImageId_{0}
        , latestError_{} { }
    ~Self() noexcept;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    struct LoadInfo {
        int32_t loadedImageId;
        int32_t numDecodedBytes    = 0U;
        int32_t width              = 0;
        int32_t height             = 0;
        int32_t numChannelsInFile  = 0;
        int32_t numChannelsDecoded = 0;
    };

    auto Load(std::string_view const filepath, int32_t numDesiredChannels) -> std::optional<LoadInfo>;
    auto Load(CpuMemory<uint8_t> encodedImageData, int32_t numDesiredChannels) -> std::optional<LoadInfo>;

    auto ImageData(int32_t loadedImageId) const -> CpuView<uint8_t const>;
    auto LatestError() const -> std::string_view { return latestError_; };

private:
    constexpr static int32_t MAX_LOADED_IMAGES = 32;
    std::unordered_map<int32_t, CpuView<uint8_t>> loadedImages_{};
    std::vector<uint8_t> temporaryBuffer_{};
    int32_t nextImageId_          = 0;
    std::string_view latestError_ = {};
};

} // namespace engine

namespace engine::gl {

namespace shader {
enum class ShaderType {
    VERTEX,
    FRAGMENT,
    COMPUTE,
};
auto LoadShaderCode
    [[nodiscard]] (std::string_view const filepath, ShaderType type, CpuView<shader::Define const> defines)
    -> std::string;
} // namespace shader

struct LoadTextureArgs final {
    ImageLoader& loader;
    std::string_view const filepath = {};
    GLenum format                   = GL_NONE;
    int32_t numChannels             = 0;
    std::string_view name           = {};
    bool withMips                   = false;
};
auto LoadTexture [[nodiscard]] (GlContext const& gl, LoadTextureArgs const& args) -> std::optional<Texture>;

} // namespace engine::gl
