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

} // namespace engine

namespace engine::gl {

auto LoadShaderCode(std::string_view const filepath, CpuView<ShaderDefine const> defines)
    -> std::string {
    std::string code = LoadTextFile(filepath);
    if (defines) {
        code             = AddShaderDefines(code, defines);
    }
    return code;
}

} // namespace engine::gl
