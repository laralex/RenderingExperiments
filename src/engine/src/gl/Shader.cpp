#include <engine/gl/Shader.hpp>
#include <engine/Assets.hpp>
#include <engine_private/Prelude.hpp>

#include <sstream>
#include <iostream>
#include <regex>

namespace {

using namespace engine::gl::shader;

void AddInclude(IncludeRegistry& out, char const* key, std::string&& text, int32_t recursionLimit) {
    out[key] = IncludeEntry {
        .text = std::move(text),
        .recursionLimit = recursionLimit,
    };
}

using ParsingIt = decltype(ShaderParsing::parts.begin());
auto WriteShaderParsing(std::stringstream& destination, ParsingIt begin, ParsingIt end, IncludeRegistry const& registry, int64_t firstOriginalLine) {
    int64_t originalLine = firstOriginalLine;
    while(begin != end) {
        switch(begin->type) {
        case ShaderParsing::PartType::ORIGINAL_CODE:
            originalLine += std::count(std::begin(begin->text), std::end(begin->text), '\n');
            destination << begin->text;
            break;
        case ShaderParsing::PartType::DELIMITER:
            //ss << "\n//" << begin->text << '\n';
            break;
        case ShaderParsing::PartType::INCLUDE_KEY:
            // TODO: conversion std::string_view to std::string
            // !! EXTRA ALLOCATIONS
            originalLine += 1;
            auto find = registry.find(std::string{begin->text});
            if (find != registry.end()) {
                destination << "// included: " << begin->text << '\n';
                destination << "#line 1 " << '\n';
                destination << find->second.text << '\n';
                destination << "#line " << originalLine << '\n';
            } else {
                destination << "// !! MISSING INCLUDE: " << begin->text << '\n';
                XLOGW("Missing shader include: {}", begin->text);
            }
            break;
        }
        ++begin;
    }
}

auto GenerateCodeRecursively[[nodiscard]](std::string_view code, IncludeRegistry const& includeRegistry, int32_t recursLimit) -> std::string {
    std::stringstream ss0, ss1;
    ss0 << code;
    auto* readStream = &ss0;
    auto* writeStream = &ss1;
    while(recursLimit-- > 0) {
        // TODO: optimize string copy here
        // clang still doesn't support C++20 readStream.view() for fuck's sake
        std::string currentCode = readStream->str(); // must outlive parsing
        auto parsing = ParseParts(currentCode);
        if (parsing.numIncludes <= 0) {
            break;
        }
        writeStream->seekp(0);
        WriteShaderParsing(*writeStream, parsing.parts.begin(), parsing.parts.end(), includeRegistry, 1);
        std::swap(readStream, writeStream);
    }
    // readStream was written last
    return readStream->str();
}

auto ExpandRegistryRecursively(IncludeRegistry& includeRegistry) {
    for(auto& kv : includeRegistry) {
        if (kv.second.recursionLimit <= 0) { continue; }
        kv.second.text = GenerateCodeRecursively(kv.second.text, includeRegistry, kv.second.recursionLimit);
    }
}

} // namespace anonymous

namespace engine::gl::shader {

void LoadCommonIncludes(IncludeRegistry& out) {
    AddInclude(out, "common/version/330", "#version 330 core", 0);
    AddInclude(out, "common/version/420", "#version 420 core",0);
    AddInclude(out, "common/consts", LoadTextFile("data/engine/shaders/include/constants.inc"), 0);
    AddInclude(out, "common/gradient_noise", LoadTextFile("data/engine/shaders/include/gradient_noise.inc"), 1);
    AddInclude(out, "common/screen_space_dither", LoadTextFile("data/engine/shaders/include/screen_space_dither.inc"), 0);
    AddInclude(out, "common/struct_light", LoadTextFile("data/engine/shaders/include/struct_light.inc"), 0);

    ExpandRegistryRecursively(out);
}

void LoadVertexIncludes(IncludeRegistry& out) {

}

void LoadFragmentIncludes(IncludeRegistry& out) {
    AddInclude(out, "frag/gradient_noise/apply",
        "   out_FragColor += (1.0 / 255.0) * GradientNoise(gl_FragCoord.xy) - (0.5 / 255.0);", 0);
    ExpandRegistryRecursively(out);
}

auto ParseParts(std::string_view code) -> ShaderParsing {
    ShaderParsing parse;
    parse.parts.reserve(64);

    auto parseEnd = 0U;
    auto push = [&](std::string_view text, ShaderParsing::PartType type) {
        if (std::size(text) == 0) { return; }
        parse.parts.push_back({text, type});
        if (type == ShaderParsing::PartType::INCLUDE_KEY) {
            ++parse.numIncludes;
        }
    };
    auto parseIncludes = [&](auto codeEnd){
        constexpr static std::string_view includeBeginPattern = "#include \"";
        // NOTE: for some reason, include line must be ended with new line
        // otherwise parsing goes wild, starts capturing last " into include path
        constexpr static std::string_view includeEndPattern = "\"\n";

        while (parseEnd < codeEnd) {
            auto includeBegin = code.find(includeBeginPattern, parseEnd);
            if (includeBegin == std::string_view::npos || includeBegin >= codeEnd) {
                break;
            }
            auto backScan = includeBegin - 1;
            while(backScan >= parseEnd && code[backScan] == ' ') {--backScan;}
            if (backScan-1 >= parseEnd && code[backScan] == '/' && code[backScan-1] == '/') {
                break; // include was commented out
            }
            auto includeEnd = code.find(includeEndPattern, includeBegin);
            if (includeEnd == std::string_view::npos || includeBegin >= codeEnd) {
                break;
            }
            push(code.substr(parseEnd, includeBegin - parseEnd), ShaderParsing::PartType::ORIGINAL_CODE);
            parseEnd = includeBegin + std::size(includeBeginPattern);
            push(code.substr(parseEnd, includeEnd - parseEnd),
                ShaderParsing::PartType::INCLUDE_KEY);
            parseEnd = includeEnd + std::size(includeEndPattern);
            while(parseEnd < codeEnd && (code[parseEnd] == ' ' || code[parseEnd] == '\n')) {++parseEnd;}
        }
        push(code.substr(parseEnd, codeEnd - parseEnd),
            ShaderParsing::PartType::ORIGINAL_CODE);
    };

    auto versionEnd = code.find('\n');
    if (versionEnd == std::string_view::npos) {
        XLOGW("Failed to parse shader version", 0);
        versionEnd = 0U;
    } else {
        versionEnd += 1;
    }
    push(code.substr(0U, versionEnd), ShaderParsing::PartType::ORIGINAL_CODE);
    push("<post_version>", ShaderParsing::PartType::DELIMITER);
    parseEnd = versionEnd;

    constexpr std::string_view mainSigBeginPattern = "void main(";
    auto mainSignatureBegin = code.find(mainSigBeginPattern, parseEnd);
    if (mainSignatureBegin == std::string_view::npos) {
        XLOGW("Failed to parse shader main signature beginning", 0);
        mainSignatureBegin = parseEnd;
    }

    parseIncludes(mainSignatureBegin);
    push("<pre_main>", ShaderParsing::PartType::DELIMITER);
    parseEnd = mainSignatureBegin;

    constexpr std::string_view mainSigEndPattern = "{";
    auto mainSignatureEnd = code.find(mainSigEndPattern, parseEnd);
    if (mainSignatureEnd == std::string_view::npos) {
        XLOGW("Failed to parse shader main signature end", 0);
        mainSignatureEnd = parseEnd;
    } else {
        mainSignatureEnd += 1;
    }

    push(code.substr(mainSignatureBegin, mainSignatureEnd - mainSignatureBegin), ShaderParsing::PartType::ORIGINAL_CODE);
    parseEnd = mainSignatureEnd;
    push("<begin_main>", ShaderParsing::PartType::DELIMITER);

    // TODO: a crutch for easier parsing, all shaders must end "main" with this pattern
    constexpr std::string_view mainBodyEndPattern = "} // main";
    auto mainBodyEnd = code.find(mainBodyEndPattern, parseEnd);
    if (mainBodyEnd == std::string_view::npos) {
        XLOGW("Failed to parse shader main body end", 0);
        mainBodyEnd = parseEnd;
    }
    parseIncludes(mainBodyEnd);
    push("<end_main>", ShaderParsing::PartType::DELIMITER);
    parseEnd = mainBodyEnd;

    parseIncludes(std::size(code));
    push("<post_main>", ShaderParsing::PartType::DELIMITER);
    parseEnd = std::size(code);

    return parse;
}

auto GenerateCode[[nodiscard]](std::string_view originalCode, IncludeRegistry const& includeRegistry, CpuView<Define const> defines) -> std::string {
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

void InjectDefines(std::stringstream& destination, CpuView<shader::Define const> defines) {
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

auto InjectDefines(std::string_view code, CpuView<shader::Define const> defines) -> std::string {
    std::stringstream ss;
    auto versionEnd = code.find('\n') + 1;
    ss << code.substr(0U, versionEnd);
    InjectDefines(ss, defines);
    ss << "#line 2\n"; // reset line counter for meaningful shader compilation errors
    ss << code.substr(versionEnd);
    return ss.str();
}

} // namespace engine::gl::shader
