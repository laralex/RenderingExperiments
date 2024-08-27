#pragma once

#include <cassert>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace engine {
template <typename T> struct CpuView;
} // namespace engine

namespace engine::gl {

struct GpuProgram;
struct ShaderDefine;
struct Vao;

auto LinkProgram
    [[nodiscard]] (std::string_view vertexShaderCode, std::string_view fragmentShaderCode, std::string_view name = {})
    -> std::optional<GpuProgram>;
auto LinkProgramFromFiles [[nodiscard]] (
    std::string_view vertexFilepath, std::string_view fragmentFilepath, CpuView<ShaderDefine const> defines,
    std::string_view name = {}) -> std::optional<GpuProgram>;

void RenderVao(Vao const&, GLenum primitive = GL_TRIANGLES);
void RenderVaoInstanced(Vao const& vao, GLsizei numInstances, GLenum primitive = GL_TRIANGLES);

// Wrapper for OpenGL object identifiers. Becomes 0 when moved away from
// This helps to define move constructor/assignment of other high level wrappers as simply "=default"
class GlHandle final {

public:
#define Self GlHandle
    explicit Self(GLuint id)
        : id_(id) { }
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&& other)
        : id_(other) {
        other.UnsafeReset();
    }
    Self& operator=(Self&& other) {
        assert(id_ == GL_NONE && "OpenGL resource leaked");
        id_ = other.id_;
        other.UnsafeReset();
        return *this;
    }
#undef Self
    void UnsafeReset() { id_ = GL_NONE; }

    // NOTE: don't implement as operator=, because it opens holes
    // with overwriting of GL object handles and leaking resources
    void UnsafeAssign(GlHandle const& other) { id_ = other.id_; }
    GLuint* operator&() { return &id_; }
    operator GLuint() const { return id_; }
    operator GLuint&() { return id_; }

private:
    GLuint id_;
};

struct alignas(16) ScreenShaderArgs final {
    float pixelsPerUnitX;
    float pixelsPerUnitY;
    float pixelsHeight;
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

auto TransformOrigin(glm::mat4& transform, bool isRowMajor = false) -> glm::vec3;
void UndoAffineScale(glm::mat4& transform);
auto UndoAffineScale(glm::mat4 const& transform) -> glm::mat4;

} // namespace engine::gl