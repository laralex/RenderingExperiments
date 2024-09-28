#pragma once

#include "engine/Precompiled.hpp"
#include <unordered_map>

namespace engine::gl::shader {

constexpr char const* VERTEX_FILE_EXTENSION = ".vert";
constexpr char const* FRAGMENT_FILE_EXTENSION = ".frag";
constexpr char const* COMPUTE_FILE_EXTENSION = ".comp";

struct IncludeEntry final {
    std::string text       = "/*NO_INCLUDE_TEXT*/";
    int64_t recursionLimit = 0;
    bool isMultiline       = true;
};

using IncludeRegistry = std::unordered_map<std::string, IncludeEntry, engine::StringHash, std::equal_to<>>;
void LoadCommonIncludes(IncludeRegistry& out);
void LoadVertexIncludes(IncludeRegistry& out);
void LoadFragmentIncludes(IncludeRegistry& out);
void LoadComputeCodegenComponents(IncludeRegistry& out);

struct ShaderParsing final {
    enum class PartType {
        ORIGINAL_CODE,
        INCLUDE_KEY,
        DELIMITER,
    };
    struct Part final {
        std::string_view text;
        PartType type;
    };
    std::vector<Part> parts;
    int64_t numIncludes = 0;
};

auto ParseParts [[nodiscard]] (std::string_view code) -> ShaderParsing;

struct Define final {
    std::string_view name = {};
    union {
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
};

auto GenerateCode
    [[nodiscard]] (std::string_view originalCode, IncludeRegistry const& includeRegistry, CpuView<Define const> defines)
    -> std::string;

auto InjectDefines [[nodiscard]] (std::string_view code, CpuView<Define const> defines) -> std::string;
void InjectDefines(std::stringstream& destination, CpuView<Define const> defines);

} // namespace engine::gl::shader
