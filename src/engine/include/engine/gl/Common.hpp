#pragma once


#include "engine/ShaderDefine.hpp"
#include "engine/CpuView.hpp"

#include <glad/gl.h>
#include <glm/mat4x4.hpp>

#include <cassert>
#include <string_view>
#include <optional>
#include <memory>
#include <vector>
#include <variant>

namespace engine {
template <typename T> struct CpuView;
} // namespace engine

namespace engine::gl::shader {

enum class ShaderType : GLenum {
    VERTEX = GL_VERTEX_SHADER,
    FRAGMENT = GL_FRAGMENT_SHADER,
    COMPUTE = GL_COMPUTE_SHADER,
};

inline auto ToShaderType[[nodiscard]](GLenum glShaderTypeEnum) -> ShaderType {
    switch (glShaderTypeEnum) {
        case GL_VERTEX_SHADER: return ShaderType::VERTEX;
        case GL_FRAGMENT_SHADER: return ShaderType::FRAGMENT;
        case GL_COMPUTE_SHADER: return ShaderType::COMPUTE;
    }
    return ShaderType::VERTEX;
}

inline auto ToShaderTypeStr[[nodiscard]](ShaderType type) -> std::string_view {
    switch (type) {
        case ShaderType::VERTEX: return "vertex";
        case ShaderType::FRAGMENT: return "fragment";
        case ShaderType::COMPUTE: return "compute";
    }
    return "unknown";
}

struct ShaderCreateInfo {
    ShaderCreateInfo(std::string&& code, ShaderType shaderType)
        : source(std::move(code))
        , compilationStage(SourceType::CODE)
        , shaderType(shaderType) {}
    ShaderCreateInfo(std::string_view filepath, ShaderType shaderType)
        : source(filepath)
        , compilationStage(SourceType::FILEPATH)
        , shaderType(shaderType) {}
    ShaderCreateInfo(GLuint id, ShaderType shaderType)
        : source(id)
        , compilationStage(SourceType::GL_ID)
        , shaderType(shaderType) {}

    void Dispose() {
        if (compilationStage != GL_ID || !std::holds_alternative<GLuint>(source)) { return; }
        glDeleteShader(std::get<GLuint>(source));
        source = GLuint{0U};
    }

    enum SourceType {
        CODE,
        FILEPATH,
        GL_ID,
    };
    std::variant<std::string, std::string_view, GLuint> source;
    SourceType compilationStage;
    ShaderType shaderType;

};

} // engine::gl::shader

namespace engine::gl {

struct GlContext;
struct GpuProgram;
struct Vao;

auto CompileGlShader [[nodiscard]] (GLenum shaderType, std::string_view code, bool logFail) -> GLuint;
void CompileShader(engine::gl::shader::ShaderCreateInfo& info, engine::CpuView<engine::ShaderDefine const> defines, bool logCode);

auto LinkProgram [[nodiscard]] (
    GlContext& gl, shader::ShaderCreateInfo vertex, shader::ShaderCreateInfo fragment,
    engine::CpuView<engine::ShaderDefine const> defines, std::string_view name = {}, bool logCode = false) -> std::optional<GpuProgram>;
auto LinkProgramFromFiles [[nodiscard]](
    GlContext& gl, std::string_view vertexFilepath, std::string_view fragmentFilepath,
    std::vector<ShaderDefine>&& defines, std::string_view name, bool logCode = false)
-> std::optional<std::shared_ptr<GpuProgram>>;

auto RelinkProgram [[nodiscard]](
    GlContext const& gl, shader::ShaderCreateInfo vertex, shader::ShaderCreateInfo fragment,
    GpuProgram const& oldProgram, CpuView<ShaderDefine const> defines, bool logCode) -> bool;

void RenderVao(Vao const&, GLenum primitive = GL_TRIANGLES);
void RenderVaoInstanced(Vao const& vao, GLuint firstInstance, GLsizei numInstances, GLenum primitive = GL_TRIANGLES);

// Wrapper for OpenGL object identifiers. Becomes 0 when moved away from
// This helps to define move constructor/assignment of other high level wrappers as simply "=default"
class GlHandle final {

public:
#define Self GlHandle
    explicit Self(GLuint id)
        : id_(id) { }
    ~Self() noexcept {
        assert(id_ == GL_NONE && "OpenGL resource leaked");
    }
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&& other)
        : id_(other) {
        other.UnsafeReset();
    }
    Self& operator=(Self&& other) {
        if (*this == other) { return *this; }
        assert(id_ == GL_NONE && "OpenGL resource leaked");
        id_ = other.id_;
        other.UnsafeReset();
        return *this;
    }
    Self& operator=(GLuint id) {
        assert(id_ == GL_NONE && "OpenGL resource leaked");
        id_ = id;
        return *this;
    }
#undef Self
    void UnsafeReset() { id_ = GL_NONE; }

    // NOTE: don't implement as operator=, because it opens holes
    // with overwriting of GL object handles and leaking resources
    void UnsafeAssign(GLuint newId) { id_ = newId; }
    GLuint const* Ptr() const { return &id_; }
    GLuint* Ptr() { return &id_; }
    operator GLuint() const { return id_; }
    operator GLuint&() { return id_; }

private:
    GLuint id_;
};

struct alignas(16) ScreenShaderArgs final {
    float pixelsPerUnitX;
    float pixelsPerUnitY;
    float pixelsWidth;
    float aspectRatio;
};

struct alignas(16) MouseShaderArgs final {
    glm::vec2 ndcPosition;
    float isPressed;
};

struct ModelViewProjShaderArgs final {
    alignas(16) glm::mat4 modelView;
    alignas(16) glm::mat4 modelViewProjection;
};

struct ViewProjShaderArgs final {
    alignas(16) glm::mat4 viewProjection;
    alignas(16) glm::mat4 invViewProjection;
};

auto TransformOrigin(glm::mat4 const& transform, bool isRowMajor = false) -> glm::vec3;
void UndoAffineScale(glm::mat4& transform);
auto UndoAffineScale(glm::mat4 const& transform) -> glm::mat4;

} // namespace engine::gl