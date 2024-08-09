#pragma once

#include <engine/Prelude.hpp>

namespace engine::gl {

struct ShaderDefine {
    std::string_view name = {};
    union {
        int32_t i32;
        uint32_t ui32;
        float f32;
        double f64;
        bool b8;
    } value;
    enum {
        INT32, UINT32, FLOAT32, FLOAT64, BOOLEAN8,
    } type;
    bool highPrecision = true;
};

auto AddShaderDefines[[nodiscard]](std::string_view code, ShaderDefine const* defines, int32_t limit, int32_t stride = 1) -> std::string;

} // namespace engine::gl
