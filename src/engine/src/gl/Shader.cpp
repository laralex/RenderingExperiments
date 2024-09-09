#include <engine/gl/Shader.hpp>
#include <engine/Assets.hpp>
#include <engine_private/Prelude.hpp>

#include <sstream>
#include <regex>

namespace {

using namespace engine::gl::shader;

void AddInclude(IncludeRegistry& out, char const* key, std::string&& text) {
    out[key] = IncludeEntry{
        .text = std::move(text),
    };
}

} // namespace anonymous

namespace engine::gl::shader {

void LoadCommonIncludes(IncludeRegistry& out) {
    AddInclude(out, "common/version/330", "#version 330 core");
    AddInclude(out, "common/version/420", "#version 420 core");
    AddInclude(out, "common/consts", LoadTextFile("data/engine/shaders/include/constants.inc"));
    AddInclude(out, "common/gradient_noise", LoadTextFile("data/engine/shaders/include/gradient_noise.inc"));
    AddInclude(out, "common/screen_space_dither", LoadTextFile("data/engine/shaders/include/screen_space_dither.inc"));
    AddInclude(out, "common/struct_light", LoadTextFile("data/engine/shaders/include/struct_light.inc"));
}

void LoadVertexIncludes(IncludeRegistry& out) {

}

void LoadFragmentIncludes(IncludeRegistry& out) {
    AddInclude(out, "frag/gradient_noise/apply",
        "   out_FragColor += (1.0 / 255.0) * GradientNoise(gl_FragCoord.xy) - (0.5 / 255.0);");
}

auto ParseParts(std::string_view code) -> ShaderParsing {
    ShaderParsing parse;
    parse.parts.reserve(64);

    auto parseEnd = 0U;
    auto parseIncludes = [&](auto codeEnd){
        constexpr std::string_view includeBeginPattern = "#include \"";
        constexpr std::string_view includeEndPattern = "\"\n";
        auto push = [&](std::string_view text, ShaderParsing::PartType type) {
            if (std::size(text) == 0) { return; }
            parse.parts.push_back({text, type});
        };
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
        }
        push(code.substr(parseEnd, codeEnd - parseEnd),
            ShaderParsing::PartType::ORIGINAL_CODE);
    };

    auto versionEnd = code.find('\n');
    if (versionEnd == std::string_view::npos) {
        XLOGW("Failed to parse shader version", 0);
        versionEnd = 0U;
    }
    versionEnd += 1;
    parse.parts.push_back({
        .text = code.substr(0U, versionEnd),
        .type = ShaderParsing::PartType::ORIGINAL_CODE,
    });
    parse.parts.push_back({
        .text = "<post_version>",
        .type = ShaderParsing::PartType::DELIMITER,
    });
    parseEnd = versionEnd;

    constexpr std::string_view mainSigBeginPattern = "void main(";
    auto mainSignatureBegin = code.find(mainSigBeginPattern, parseEnd);
    if (mainSignatureBegin == std::string_view::npos) {
        XLOGW("Failed to parse shader main signature beginning", 0);
        mainSignatureBegin = parseEnd;
    }

    parseIncludes(mainSignatureBegin);
    parse.parts.push_back({
        .text = "<pre_main>",
        .type = ShaderParsing::PartType::DELIMITER,
    });
    parseEnd = mainSignatureBegin;

    constexpr std::string_view mainSigEndPattern = "{";
    auto mainSignatureEnd = code.find(mainSigEndPattern, parseEnd);
    if (mainSignatureEnd == std::string_view::npos) {
        XLOGW("Failed to parse shader main signature end", 0);
        mainSignatureEnd = parseEnd;
    }
    mainSignatureEnd += 1;

    parse.parts.push_back({
        .text = code.substr(mainSignatureBegin, mainSignatureEnd - mainSignatureBegin),
        .type = ShaderParsing::PartType::ORIGINAL_CODE, // main signature
    });
    parseEnd = mainSignatureEnd;
    parse.parts.push_back({
        .text = "<begin_main>",
        .type = ShaderParsing::PartType::DELIMITER,
    });

    // TODO: a crutch for easier parsing, all shaders must end "main" with this pattern
    constexpr std::string_view mainBodyEndPattern = "} // main";
    auto mainBodyEnd = code.find(mainBodyEndPattern, parseEnd);
    if (mainBodyEnd == std::string_view::npos) {
        XLOGW("Failed to parse shader main body end", 0);
        mainBodyEnd = parseEnd;
    }
    parseIncludes(mainBodyEnd);
    parse.parts.push_back({
        .text = "<end_main>",
        .type = ShaderParsing::PartType::DELIMITER,
    });
    parseEnd = mainBodyEnd;

    parseIncludes(std::size(code));
    parse.parts.push_back({
        .text = "<post_main>",
        .type = ShaderParsing::PartType::DELIMITER,
    });
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

    int64_t originalLine = 2;
    while(partIt != parsing.parts.end()) {
        switch(partIt->type) {
        case ShaderParsing::PartType::ORIGINAL_CODE:
            originalLine += std::count(std::begin(partIt->text), std::end(partIt->text), '\n');
            ss << partIt->text;
            break;
        case ShaderParsing::PartType::DELIMITER:
            //ss << "\n//" << partIt->text << '\n';
            break;
        case ShaderParsing::PartType::INCLUDE_KEY:
            // TODO: conversion std::string_view to std::string
            // !! EXTRA ALLOCATIONS
            originalLine += 1;
            auto find = includeRegistry.find(std::string{partIt->text});
            if (find != includeRegistry.end()) {
                ss << "#line 1 " << '\n';
                ss << "// begin include: " << partIt->text << '\n';
                ss << find->second.text << '\n';
                ss << "// end include: " << partIt->text << '\n';
                ss << "#line " << originalLine << '\n';
            } else {
                ss << "// !! MISSING INCLUDE: " << partIt->text << '\n';
                XLOGW("Missing shader include: {}", partIt->text);
            }
            break;
        }
        ++partIt;
    }

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
