#include "engine/Assets.hpp"
#include "engine_private/Prelude.hpp"

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

} // namespace engine

namespace engine::gl {

auto LoadShaderCode(std::string_view const filepath, CpuView<ShaderDefine const> defines) -> std::string {
    std::string code = LoadTextFile(filepath);
    if (defines) { code = AddShaderDefines(code, defines); }
    return code;
}

} // namespace engine::gl
