#include "engine/gl/Shader.hpp"
#include "engine/Assets.hpp"

#include "engine_private/Prelude.hpp"

#include <iostream>
#include <regex>
#include <sstream>

namespace {

using namespace engine::gl::shader;

constexpr int64_t INCLUDE_LINE_NUMBER_BASE = 1'000'000;

void AddInclude(
    IncludeRegistry& out, char const* key, std::string&& text, int32_t recursionLimit, bool isMultiline = true) {
    out[key] = IncludeEntry{
        .text           = std::move(text),
        .recursionLimit = recursionLimit,
        .isMultiline    = isMultiline,
    };
}

using ParsingIt = decltype(ShaderParsing::parts.begin());
auto WriteShaderParsing(
    std::stringstream& destination, ParsingIt begin, ParsingIt end, IncludeRegistry const& registry,
    int64_t firstOriginalLine) {
    int64_t originalLine = firstOriginalLine;
    int64_t includeCount = 0;
    bool isFirstInclude  = firstOriginalLine < INCLUDE_LINE_NUMBER_BASE;
    while (begin != end) {
        switch (begin->type) {
        case ShaderParsing::PartType::ORIGINAL_CODE:
            originalLine += std::count(std::begin(begin->text), std::end(begin->text), '\n');
            destination << begin->text;
            break;
        case ShaderParsing::PartType::DELIMITER:
            // ss << "\n//" << begin->text << '\n';
            break;
        case ShaderParsing::PartType::INCLUDE_KEY:
            // TODO: conversion std::string_view to std::string
            // !! EXTRA ALLOCATIONS
            originalLine += 1;
            ++includeCount;
            auto find = registry.find(std::string{begin->text});
            if (find == registry.end()) {
                destination << "// !! MISSING INCLUDE IN REGISTRY: " << begin->text << '\n';
                XLOGW("Missing shader include in registry: {}", begin->text);
                break;
            }
            auto const& expandedInclude = find->second;
            if (expandedInclude.text.size() == 0) {
                destination << "// !! EMPTY INCLUDE TEXT: " << begin->text << '\n';
                XLOGW("Empty shader include text of: {}", begin->text);
                break;
            }
            if (expandedInclude.isMultiline) { destination << "// included: " << begin->text << '\n'; }
            if (expandedInclude.isMultiline && isFirstInclude) {
                destination << "#line " << includeCount * INCLUDE_LINE_NUMBER_BASE << '\n';
            }
            destination << expandedInclude.text;
            if (expandedInclude.isMultiline && isFirstInclude) {
                destination << '\n' << "#line " << originalLine << '\n';
            }
            break;
        }
        ++begin;
    }
}

auto GenerateCodeRecursively
    [[nodiscard]] (std::string_view code, IncludeRegistry const& includeRegistry, int32_t recursLimit) -> std::string {
    std::stringstream ss0, ss1;
    ss0 << code;
    auto* readStream  = &ss0;
    auto* writeStream = &ss1;
    while (recursLimit-- > 0) {
        // TODO: optimize string copy here
        // clang still doesn't support C++20 readStream.view() for fuck's sake
        std::string currentCode = readStream->str(); // must outlive parsing
        auto parsing            = ParseParts(currentCode);
        if (parsing.numIncludes <= 0) { break; }
        writeStream->seekp(0);
        WriteShaderParsing(
            *writeStream, parsing.parts.begin(), parsing.parts.end(), includeRegistry, INCLUDE_LINE_NUMBER_BASE);
        std::swap(readStream, writeStream);
    }
    // readStream was written last
    return readStream->str();
}

auto ExpandRegistryRecursively(IncludeRegistry& includeRegistry) {
    for (auto& kv : includeRegistry) {
        if (kv.second.recursionLimit <= 0) { continue; }
        kv.second.text = GenerateCodeRecursively(kv.second.text, includeRegistry, kv.second.recursionLimit);
    }
}

} // namespace

namespace engine::gl::shader {

ENGINE_EXPORT void LoadCommonIncludes(IncludeRegistry& out) {
    AddInclude(out, "common/version/330", "#version 330 core", 0);
    AddInclude(out, "common/version/420", "#version 420 core", 0);
    AddInclude(out, "common/consts", LoadTextFile("data/engine/shaders/include/constants.inc"), 0);
    AddInclude(out, "common/gradient_noise", LoadTextFile("data/engine/shaders/include/gradient_noise.inc"), 1);
    AddInclude(
        out, "common/screen_space_dither", LoadTextFile("data/engine/shaders/include/screen_space_dither.inc"), 0);
    AddInclude(out, "common/struct/light", LoadTextFile("data/engine/shaders/include/struct_light.inc"), 0);
    AddInclude(out, "common/struct/material", LoadTextFile("data/engine/shaders/include/struct_material.inc"), 0);
    AddInclude(out, "common/ubo/material", LoadTextFile("data/engine/shaders/include/ubo_material.inc"), 1);

    ExpandRegistryRecursively(out);
}

ENGINE_EXPORT void LoadVertexIncludes(IncludeRegistry& out) { }

ENGINE_EXPORT void LoadFragmentIncludes(IncludeRegistry& out) {
    AddInclude(
        out, "frag/gradient_noise/eval", "(1.0 / 255.0) * GradientNoise(gl_FragCoord.xy) - (0.5 / 255.0)", 0, false);
    ExpandRegistryRecursively(out);
}

ENGINE_EXPORT auto ParseParts(std::string_view code) -> ShaderParsing {
    ShaderParsing parse;
    parse.parts.reserve(64);

    auto parseEnd = 0U;
    auto push     = [&](std::string_view text, ShaderParsing::PartType type) {
        if (std::size(text) == 0) { return; }
        parse.parts.push_back({text, type});
        if (type == ShaderParsing::PartType::INCLUDE_KEY) { ++parse.numIncludes; }
    };
    auto parseIncludes = [&](auto codeEnd) {
        constexpr static std::string_view includeBeginPattern = "#include \"";
        // NOTE: for some reason, include line must be ended with new line
        // otherwise parsing goes wild, starts capturing last " into include path
        constexpr static std::string_view includeEndPattern = "\"";

        while (parseEnd < codeEnd) {
            auto includeBegin = code.find(includeBeginPattern, parseEnd);
            if (includeBegin == std::string_view::npos || includeBegin >= codeEnd) { break; }
            auto backScan = includeBegin - 1;
            while (backScan >= parseEnd && code[backScan] == ' ') {
                --backScan;
            }
            if (backScan - 1 >= parseEnd && code[backScan] == '/' && code[backScan - 1] == '/') {
                break; // include was commented out
            }
            auto includeEnd = code.find(includeEndPattern, includeBegin + std::size(includeBeginPattern));
            if (includeEnd == std::string_view::npos || includeBegin >= codeEnd) { break; }
            push(code.substr(parseEnd, includeBegin - parseEnd), ShaderParsing::PartType::ORIGINAL_CODE);
            parseEnd = includeBegin + std::size(includeBeginPattern);
            push(code.substr(parseEnd, includeEnd - parseEnd), ShaderParsing::PartType::INCLUDE_KEY);
            parseEnd = includeEnd + std::size(includeEndPattern);
            while (parseEnd < codeEnd && (code[parseEnd] == ' ' /* || code[parseEnd] == '\n' */)) {
                ++parseEnd;
            }
        }
        push(code.substr(parseEnd, codeEnd - parseEnd), ShaderParsing::PartType::ORIGINAL_CODE);
    };

    auto versionEnd = code.find("#version");
    if (versionEnd == std::string_view::npos) {
        XLOGW("Failed to parse shader version", 0);
        versionEnd = 0U;
    } else {
        versionEnd = code.find('\n', versionEnd) + 1;
    }
    push(code.substr(0U, versionEnd), ShaderParsing::PartType::ORIGINAL_CODE);
    push("<post_version>", ShaderParsing::PartType::DELIMITER);
    parseEnd = versionEnd;

    parseIncludes(std::size(code));
    parseEnd = std::size(code);

    return parse;
}

ENGINE_EXPORT auto GenerateCode
    [[nodiscard]] (std::string_view originalCode, IncludeRegistry const& includeRegistry, CpuView<Define const> defines)
    -> std::string {
    auto parsing = shader::ParseParts(originalCode);
    std::stringstream ss;
    auto partIt = parsing.parts.begin();

    assert(partIt->type == ShaderParsing::PartType::ORIGINAL_CODE);
    ss << partIt->text;
    ++partIt;
    assert(partIt->type == ShaderParsing::PartType::DELIMITER && partIt->text == "<post_version>");
    InjectDefines(ss, defines);
    ss << "#line 2\n";
    WriteShaderParsing(ss, partIt, parsing.parts.end(), includeRegistry, 2);
    return ss.str();
}

ENGINE_EXPORT void InjectDefines(std::stringstream& destination, CpuView<shader::Define const> defines) {
    if constexpr (engine::DEBUG_BUILD) { destination << "#define DEBUG 1\n"; }
    size_t numDefines = defines.NumElements();
    for (size_t i = 0; i < numDefines; ++i) {
        shader::Define const& define = *defines[i];
        destination << "#define" << ' ' << define.name << ' ';
        switch (define.type) {
        case shader::Define::INT32:
            destination << define.value.i32;
            break;
        case shader::Define::UINT32:
            destination << define.value.ui32;
            break;
        case shader::Define::FLOAT32:
            destination << define.value.f32;
            break;
        case shader::Define::FLOAT64:
            destination << define.value.f64;
            break;
        case shader::Define::BOOLEAN8:
            destination << define.value.b8;
            break;
        }
        destination << '\n';
    }
}

ENGINE_EXPORT auto InjectDefines(std::string_view code, CpuView<shader::Define const> defines) -> std::string {
    std::stringstream ss;
    auto versionEnd = code.find('\n') + 1;
    ss << code.substr(0U, versionEnd);
    InjectDefines(ss, defines);
    ss << "#line 2\n"; // reset line counter for meaningful shader compilation errors
    ss << code.substr(versionEnd);
    return ss.str();
}

} // namespace engine::gl::shader
