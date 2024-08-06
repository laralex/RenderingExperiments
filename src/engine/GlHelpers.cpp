#include "engine/GlHelpers.hpp"
#include "engine/GlTextureUnits.hpp"
#include "engine/GlBuffer.hpp"
#include "engine/GlVao.hpp"
#include "engine_private/Prelude.hpp"

namespace {

void GLAPIENTRY DebugOutputCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
    const void* userParam) {
    auto logLevel = spdlog::level::level_enum::warn;
    if (type == GL_DEBUG_TYPE_ERROR) { logLevel = spdlog::level::level_enum::err; }
    XLOG_LVL(
        logLevel, "GL_KHR_debug (type,source,severity)=(0x{:x},0x{:x},0x{:x}), message = {}", type, source, severity,
        message);
}

} // namespace

namespace engine::gl {

ENGINE_EXPORT void InitializeGl() {
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

void CheckOpenGLError(const char* stmt, const char* fname, int line, bool fatal) {
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
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLCALL(glObjectLabel(GL_BUFFER, buffer.Id(), label.size(), label.data()));
    }
}

void GetDebugLabel(GpuBuffer const& buffer, char* outBuffer, size_t outBufferSize) {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLsizei bytesWritten = 0;
        GLCALL(glGetObjectLabel(GL_BUFFER, buffer.Id(), outBufferSize, &bytesWritten, outBuffer));
    }
}

void DebugLabel(Vao const& vertexArrayObject, std::string_view label) {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLCALL(glObjectLabel(GL_VERTEX_ARRAY, vertexArrayObject.Id(), label.size(), label.data()));
    }
}

void GetDebugLabel(Vao const& vertexArrayObject, char* outBuffer, size_t outBufferSize) {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLsizei bytesWritten = 0;
        GLCALL(glGetObjectLabel(GL_VERTEX_ARRAY, vertexArrayObject.Id(), outBufferSize, &bytesWritten, outBuffer));
    }
}

void DebugLabel(void* glSyncObject, std::string_view label) {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLCALL(glObjectPtrLabel(glSyncObject, label.size(), label.data()));
    }
}

void GetDebugLabel(void* glSyncObject, char* outBuffer, size_t outBufferSize) {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLsizei bytesWritten = 0;
        GLCALL(glGetObjectPtrLabel(glSyncObject, outBufferSize, &bytesWritten, outBuffer));
    }
}

} // namespace engine::gl