#include <engine/gl/Shader.hpp>
#include <engine_private/Prelude.hpp>

#include <sstream>

namespace engine::gl {

auto AddShaderDefines(std::string_view code, CpuView<ShaderDefine const> defines)
    -> std::string {
    std::stringstream ss;
    auto versionEnd = code.find('\n') + 1;
    ss << code.substr(0U, versionEnd);
    size_t numDefines = defines.NumElements();
    for (size_t i = 0; i < numDefines; ++i) {
        ShaderDefine const& define = *defines[i];
        ss << "#define" << ' ' << define.name << ' ';
        switch (define.type) {
        case ShaderDefine::INT32:
            ss << define.value.i32;
            break;
        case ShaderDefine::UINT32:
            ss << define.value.ui32;
            break;
        case ShaderDefine::FLOAT32:
            ss << define.value.f32;
            break;
        case ShaderDefine::FLOAT64:
            ss << define.value.f64;
            break;
        case ShaderDefine::BOOLEAN8:
            ss << define.value.b8;
            break;
        }
        ss << '\n';
    }
    XLOG("Shader defines: {}", ss.str());
    ss << code.substr(versionEnd);
    return ss.str();
}

} // namespace engine::gl
