#include "engine/GlHelpers.hpp"
#include "engine/GlBuffer.hpp"
#include "engine/GlTextureUnits.hpp"
#include "engine/GlVao.hpp"
#include "engine_private/Prelude.hpp"

namespace {

void GLAPIENTRY DebugOutputCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
    const void* userParam) {
#ifdef XDEBUG
    auto logLevel = spdlog::level::level_enum::warn;
    if (type == GL_DEBUG_TYPE_ERROR) { logLevel = spdlog::level::level_enum::err; }
    XLOG_LVL(
        logLevel, "GL_KHR_debug (type,source,severity)=(0x{:x},0x{:x},0x{:x}), message = {}", type, source, severity,
        message);
#endif
}

void DebugLabel(GLenum objectTypeKhr, GLenum objectTypeExt, GLuint objectId, std::string_view label) {
    using engine::gl::GlExtensions;
    assert(GlExtensions::IsInitialized());
    // Debug labels support
    // + GL_BUFFER
    // GL_SHADER
    // GL_PROGRAM
    // + GL_VERTEX_ARRAY
    // GL_QUERY
    // GL_PROGRAM_PIPELINE
    // GL_TRANSFORM_FEEDBACK
    // GL_SAMPLER
    // GL_TEXTURE
    // GL_RENDERBUFFER
    // GL_FRAMEBUFFER
    // + sync objects (pointers)
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLCALL(glObjectLabel(objectTypeKhr, objectId, label.size(), label.data()));
        return;
    }
    // + BUFFER_OBJECT_EXT                              0x9151
    // SHADER_OBJECT_EXT                              0x8B48
    // PROGRAM_OBJECT_EXT                             0x8B40
    // + VERTEX_ARRAY_OBJECT_EXT                        0x9154
    // QUERY_OBJECT_EXT                               0x9153
    // PROGRAM_PIPELINE_OBJECT_EXT                    0x8A4F
    if (GlExtensions::Supports(GlExtensions::EXT_debug_label) && objectTypeExt != GL_NONE) {
        GLCALL(glLabelObjectEXT(objectTypeExt, objectId, label.size(), label.data()));
    }
}

auto GetDebugLabel
    [[nodiscard]] (GLenum objectTypeKhr, GLenum objectTypeExt, GLuint objectId, char* outBuffer, size_t outBufferSize)
    -> GLsizei {
    using engine::gl::GlExtensions;
    assert(GlExtensions::IsInitialized());
    GLsizei bytesWritten = 0U;
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLCALL(glGetObjectLabel(objectTypeKhr, objectId, outBufferSize, &bytesWritten, outBuffer));
        return static_cast<size_t>(bytesWritten);
    }
    if (GlExtensions::Supports(GlExtensions::EXT_debug_label) && objectTypeExt != GL_NONE) {
        GLCALL(glGetObjectLabelEXT(objectTypeExt, objectId, outBufferSize, &bytesWritten, outBuffer));
        return static_cast<size_t>(bytesWritten);
    }
    return 0U;
}

} // namespace

namespace engine::gl {

ENGINE_EXPORT void InitializeOpenGl() {
    gl::GlExtensions::Initialize();
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLCALL(glEnable(GL_DEBUG_OUTPUT));
        void const* userParam = nullptr;
        GLCALL(glDebugMessageCallback(DebugOutputCallback, userParam));
        GLCALL(glDebugMessageControl(
            /*source*/ GL_DONT_CARE,
            /*type*/ GL_DONT_CARE,
            /*severity*/ GL_DEBUG_SEVERITY_NOTIFICATION,
            /*count*/ 0,
            /*ids*/ nullptr,
            /*enabled*/ false));
    }

    gl::GlCapabilities::Initialize();
    gl::GlTextureUnits::Initialize();
}

void CheckOpenGlError(const char* stmt, const char* fname, int line, bool fatal) {
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) { return; }
    char const* errLabel = "UNKNOWN_ERROR";
    switch (err) {
    case GL_INVALID_ENUM:
        errLabel = "GL_INVALID_ENUM";
        break;
    case GL_INVALID_VALUE:
        errLabel = "GL_INVALID_VALUE";
        break;
    case GL_INVALID_OPERATION:
        errLabel = "GL_INVALID_OPERATION";
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        errLabel = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
    }
    XLOGE("{}(0x{:x}), at {}:{} - for {}\n", errLabel, err, fname, line, stmt);
    if (fatal) { std::terminate(); }
}

void DebugLabel(GpuBuffer const& buffer, std::string_view label) {
    ::DebugLabel(GL_BUFFER, GL_BUFFER_OBJECT_EXT, buffer.Id(), label);
}

auto GetDebugLabel(GpuBuffer const& buffer, char* outBuffer, size_t outBufferSize) -> size_t {
    return ::GetDebugLabel(GL_BUFFER, GL_BUFFER_OBJECT_EXT, buffer.Id(), outBuffer, outBufferSize);
}

void DebugLabel(Vao const& vertexArrayObject, std::string_view label) {
    ::DebugLabel(GL_VERTEX_ARRAY, GL_VERTEX_ARRAY_OBJECT_EXT, vertexArrayObject.Id(), label);
}

auto GetDebugLabel(Vao const& vertexArrayObject, char* outBuffer, size_t outBufferSize) -> size_t {
    return ::GetDebugLabel(
        GL_VERTEX_ARRAY, GL_VERTEX_ARRAY_OBJECT_EXT, vertexArrayObject.Id(), outBuffer, outBufferSize);
}

void DebugLabel(void* glSyncObject, std::string_view label) {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLCALL(glObjectPtrLabel(glSyncObject, label.size(), label.data()));
    }
}

auto GetDebugLabel(void* glSyncObject, char* outBuffer, size_t outBufferSize) -> size_t {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLsizei bytesWritten = 0;
        GLCALL(glGetObjectPtrLabel(glSyncObject, outBufferSize, &bytesWritten, outBuffer));
        return static_cast<size_t>(bytesWritten);
    }
    return 0U;
}

} // namespace engine::gl