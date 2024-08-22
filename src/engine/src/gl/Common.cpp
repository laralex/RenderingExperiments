#include "engine/Assets.hpp"
#include "engine/Prelude.hpp"
#include "engine/gl/Vao.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Shader.hpp"
#include "engine_private/Prelude.hpp"

namespace engine::gl {

auto LinkProgram(std::string_view vertexShaderCode, std::string_view fragmentShaderCode, std::string_view name)
    -> std::optional<GpuProgram> {
    GLuint vertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderCode);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);

    auto maybeProgram = GpuProgram::Allocate(vertexShader, fragmentShader, name);
    assert(maybeProgram);

    GLCALL(glDeleteShader(vertexShader));
    GLCALL(glDeleteShader(fragmentShader));

    return maybeProgram;
}

auto LinkProgramFromFiles(
    std::string_view vertexFilepath, std::string_view fragmentFilepath, CpuView<ShaderDefine const> defines,
    std::string_view name) -> std::optional<GpuProgram> {
    std::string vertexShaderCode   = LoadShaderCode(vertexFilepath, defines);
    std::string fragmentShaderCode = LoadShaderCode(fragmentFilepath, defines);
    return LinkProgram(vertexShaderCode, fragmentShaderCode, name);
}

void RenderVao(Vao const& vao, GLenum primitive) {
    auto vaoGuard = VaoCtx{vao};
    GLint firstIndex = vao.FirstIndex();
    if (vao.IsIndexed()) {
        // TODO: crash when passing firstIndex instead of 0, the static buffers get updated
        GLCALL(glDrawElements(primitive, vao.IndexCount(), vao.IndexDataType(), 0));
    } else {
        GLCALL(glDrawArrays(primitive, firstIndex, vao.IndexCount()));
    }
}

} // namespace engine::gl