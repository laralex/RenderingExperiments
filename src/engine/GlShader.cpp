#include <engine/GlShader.hpp>
#include <engine_private/Prelude.hpp>

#include <sstream>

namespace engine::gl {

auto AddShaderDefines(std::string_view code, ShaderDefine const* defines, int32_t limit, int32_t stride)
    -> std::string {
    std::stringstream ss;
    auto versionEnd = code.find('\n') + 1;
    ss << code.substr(0U, versionEnd);
    for (size_t i = 0; i < limit; i += stride) {
        ShaderDefine const& define = defines[i];
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
    ss << code.substr(versionEnd);
    return ss.str();
}

} // namespace engine::gl
