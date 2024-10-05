#pragma once

#include <string_view>

namespace engine {

enum GpuProgramType {
    GRAPHICAL,
    COMPUTE,
};

struct ShaderDefine final {
    std::string_view name = {};
    union Value {
        int32_t i32;
        uint32_t ui32;
        float f32;
        double f64;
        bool b8;
    } value;
    enum {
        INT32,
        UINT32,
        FLOAT32,
        FLOAT64,
        BOOLEAN8,
    } type;
    bool highPrecision = true;
    static auto I32 [[nodiscard]](std::string_view name, int32_t value) -> ShaderDefine {
        auto d = ShaderDefine{.name = name, .type = INT32, .highPrecision = true};
        d.value.i32 = value;
        return d;
    }
    static auto UI32 [[nodiscard]](std::string_view name, uint32_t value) -> ShaderDefine {
        auto d = ShaderDefine{.name = name, .type = UINT32, .highPrecision = true};
        d.value.ui32 = value;
        return d;
    }
    static auto F32 [[nodiscard]](std::string_view name, float value) -> ShaderDefine {
        auto d = ShaderDefine{.name = name, .type = FLOAT32, .highPrecision = true};
        d.value.f32 = value;
        return d;
    }
    static auto F64 [[nodiscard]](std::string_view name, double value) -> ShaderDefine {
        auto d = ShaderDefine{.name = name, .type = FLOAT64, .highPrecision = true};
        d.value.f64 = value;
        return d;
    }
    static auto B8 [[nodiscard]](std::string_view name, bool value) -> ShaderDefine {
        auto d = ShaderDefine{.name = name, .type = BOOLEAN8, .highPrecision = true};
        d.value.b8 = value;
        return d;
    }
};

} // namespace engine