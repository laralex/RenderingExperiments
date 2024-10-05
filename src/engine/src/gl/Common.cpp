#include "engine/gl/Common.hpp"
#include "engine/Assets.hpp"
#include "engine/Precompiled.hpp"
#include "engine/gl/GpuProgram.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Vao.hpp"

#include "engine_private/Prelude.hpp"

namespace {

void LogShaderCode(std::string_view msg, std::string_view vertexShaderCode, engine::gl::shader::ShaderType type, bool doLog) {
    using engine::gl::shader::ShaderType;
    if (doLog) {
        XLOG("{} type={}\n{}", msg, engine::gl::shader::ToShaderTypeStr(type), vertexShaderCode);
    }
}

auto AllocateGraphicalShaders
    [[nodiscard]] (std::string_view vertexShaderCode, std::string_view fragmentShaderCode, bool logCode)
    -> std::pair<GLuint, GLuint> {
    LogShaderCode("Compiling shader", vertexShaderCode, engine::gl::shader::ShaderType::VERTEX, logCode);
    LogShaderCode("Compiling shader", fragmentShaderCode, engine::gl::shader::ShaderType::FRAGMENT, logCode);
    GLuint vertexShader   = engine::gl::CompileShader(GL_VERTEX_SHADER, vertexShaderCode, logCode);
    GLuint fragmentShader = engine::gl::CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode, logCode);
    return {vertexShader, fragmentShader};
}

} // namespace

namespace engine::gl {

ENGINE_EXPORT auto CompileShader(GLenum shaderType, std::string_view code, bool logFail) -> GLuint {
    GLenum shader;
    GLCALL(shader = glCreateShader(shaderType));

    char const* codeRaw  = code.data();
    GLint const codeSize = code.size();
    GLCALL(glShaderSource(shader, 1, &codeRaw, &codeSize));

    GLCALL(glCompileShader(shader));

    GLint isCompiled;
    GLCALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled));
    if (isCompiled == GL_TRUE) {
        // XLOG("Compiled shader {}", shader);
        return shader; // success
    }

    static char infoLog[512];
    GLCALL(glGetShaderInfoLog(shader, 512, nullptr, infoLog));
    GLCALL(glDeleteShader(shader));
    LogShaderCode("Failed to compile shader", code, shader::ToShaderType(shaderType), logFail);
    return GL_NONE;
}

ENGINE_EXPORT void CompileShader(shader::ShaderCreateInfo& info, CpuView<ShaderDefine const> defines, bool logCode) {
    using shader::ShaderCreateInfo;
    GLuint shader_id = GL_NONE;
    switch (info.compilationStage) {
        case ShaderCreateInfo::FILEPATH:
            info.compilationStage = ShaderCreateInfo::CODE;
            info.source = LoadShaderCode(std::get<std::string_view>(info.source), info.shaderType, defines);
            LogShaderCode("Compiling shader", std::get<std::string>(info.source), info.shaderType, logCode);
            CompileShader(info, defines, logCode);
            break;
        case ShaderCreateInfo::CODE:
            shader_id = CompileShader(static_cast<GLenum>(info.shaderType), std::get<std::string>(info.source), logCode);
            assert(shader_id > 0);
            info.compilationStage = ShaderCreateInfo::GL_ID;
            info.source = shader_id;
            break;
        case ShaderCreateInfo::GL_ID:
            return;
        default:
            return;
    }
}


ENGINE_EXPORT auto LinkProgram(
    GlContext& gl, std::string_view vertexShaderCode, std::string_view fragmentShaderCode, std::string_view name,
    bool logCode) -> std::optional<GpuProgram> {

    auto [vertexShader, fragmentShader] = AllocateGraphicalShaders(vertexShaderCode, fragmentShaderCode, logCode);
    auto maybeProgram                   = GpuProgram::Allocate(gl, vertexShader, fragmentShader, name);

    GLCALL(glDeleteShader(vertexShader));
    GLCALL(glDeleteShader(fragmentShader));

    return maybeProgram;
}

ENGINE_EXPORT auto LinkProgramFromFiles(
    GlContext& gl, std::string_view vertexFilepath, std::string_view fragmentFilepath,
    std::vector<ShaderDefine>&& defines, std::string_view name, bool logCode) -> std::optional<std::shared_ptr<GpuProgram>> {
    auto definesView = CpuView{defines.data(), std::size(defines)};
    auto vert = shader::ShaderCreateInfo(vertexFilepath, shader::ShaderType::VERTEX);
    auto frag = shader::ShaderCreateInfo(fragmentFilepath, shader::ShaderType::FRAGMENT);
    CompileShader(vert, definesView, logCode);
    CompileShader(frag, definesView, logCode);
    auto maybeProgram = GpuProgram::Allocate(gl, std::get<GLuint>(vert.source), std::get<GLuint>(frag.source), name);
    if (!maybeProgram) { return std::nullopt; }

    auto program = std::make_shared<GpuProgram>(std::move(*maybeProgram));
    gl.Programs()->RegisterProgram(program, vertexFilepath, fragmentFilepath, std::move(defines));
    return std::optional{program};
}

ENGINE_EXPORT auto RelinkProgram(
    GlContext const& gl, std::string_view vertexShaderCode, std::string_view fragmentShaderCode,
    GpuProgram const& oldProgram, bool logCode) -> bool {
    auto [vertexShader, fragmentShader] = AllocateGraphicalShaders(vertexShaderCode, fragmentShaderCode, logCode);
    // TODO: also fail if any shader didn't compile
    if (vertexShader <= 0 || fragmentShader <= 0) {
        GLCALL(glDeleteShader(vertexShader));
        GLCALL(glDeleteShader(fragmentShader));
        return false;
    }
    constexpr bool isRecompile = true;
    bool ok                    = oldProgram.LinkGraphical(vertexShader, fragmentShader, isRecompile);
    GLCALL(glDeleteShader(vertexShader));
    GLCALL(glDeleteShader(fragmentShader));
    return ok;
}

ENGINE_EXPORT auto RelinkProgramFromFiles(
    GlContext const& gl, std::string_view vertexFilepath, std::string_view fragmentFilepath,
    CpuView<ShaderDefine const> defines, GpuProgram const& oldProgram, bool logCode) -> bool {
    std::string vertexShaderCode   = LoadShaderCode(vertexFilepath, shader::ShaderType::VERTEX, defines);
    std::string fragmentShaderCode = LoadShaderCode(fragmentFilepath, shader::ShaderType::FRAGMENT, defines);
    return RelinkProgram(gl, vertexShaderCode, fragmentShaderCode, oldProgram, logCode);
}

ENGINE_EXPORT void RenderVao(Vao const& vao, GLenum primitive) {
    auto vaoGuard      = VaoCtx{vao};
    GLint firstIndex   = vao.FirstIndex();
    GLsizei numIndices = vao.IndexCount();
    if (vao.IsIndexed()) {
        auto const* firstIndexPtr = reinterpret_cast<decltype(firstIndex) const*>(firstIndex);
        GLCALL(glDrawElements(primitive, numIndices, vao.IndexDataType(), firstIndexPtr));
    } else {
        GLCALL(glDrawArrays(primitive, firstIndex, numIndices));
    }
}

ENGINE_EXPORT void RenderVaoInstanced(Vao const& vao, GLuint firstInstance, GLsizei numInstances, GLenum primitive) {
    auto vaoGuard      = VaoCtx{vao};
    GLint firstIndex   = vao.FirstIndex();
    GLsizei numIndices = vao.IndexCount();
    if (vao.IsIndexed()) {
        auto const* firstIndexPtr = reinterpret_cast<decltype(firstIndex) const*>(firstIndex);
        GLCALL(glDrawElementsInstancedBaseInstance(
            primitive, numIndices, vao.IndexDataType(), firstIndexPtr, numInstances, firstInstance));
    } else {
        GLCALL(glDrawArraysInstancedBaseInstance(primitive, firstIndex, numIndices, numInstances, firstInstance));
    }
}

ENGINE_EXPORT auto TransformOrigin(glm::mat4 const& transform, bool isRowMajor) -> glm::vec3 {
    if (isRowMajor) { return glm::vec3{transform[0][3], transform[1][3], transform[2][3]}; }
    return glm::vec3{transform[3][0], transform[3][1], transform[3][2]};
}

ENGINE_EXPORT void UndoAffineScale(glm::mat4& transform) {
    auto invThenTransposed = glm::inverse(transform);
    invThenTransposed      = glm::transpose(invThenTransposed);
    transform              = invThenTransposed * transform;
}

ENGINE_EXPORT auto UndoAffineScale(glm::mat4 const& transform) -> glm::mat4 {
    auto invThenTransposed = glm::inverse(transform);
    invThenTransposed      = glm::transpose(invThenTransposed);
    return invThenTransposed * transform;
}

} // namespace engine::gl