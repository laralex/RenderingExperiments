#include "engine/GlBuffer.hpp"
#include "engine/GlProgram.hpp"
#include "engine/GlVao.hpp"
#include "engine/Prelude.hpp"
#include "engine_private/Prelude.hpp"

namespace {

constexpr bool ENABLE_KHR_DEBUG_CALLBACK = false;
constexpr bool ENABLE_DEBUG_GROUP_LOGS   = false;

} // namespace

namespace {

void GLAPIENTRY DebugOutputCallback(
    GLenum source, GLenum type, GLuint /*id*/, GLenum severity, GLsizei length, const GLchar* message,
    const void* userParam) {
#ifdef XDEBUG
    auto logLevel = spdlog::level::level_enum::warn;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        logLevel = spdlog::level::level_enum::err;
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
    case GL_DEBUG_TYPE_PUSH_GROUP:
        logLevel = spdlog::level::level_enum::info;
        break;
    }
    if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_ERROR_ARB) { }
    XLOG_LVL(logLevel, "GL_KHR_debug src=0x{:x} severe=0x{:x}: {}", source, severity, message);
#endif
}

void DebugLabel(GLenum objectTypeKhr, GLenum objectTypeExt, GLuint objectId, std::string_view label) {
    using engine::gl::GlExtensions;
    assert(GlExtensions::IsInitialized());
    // Debug labels support
    // + GL_BUFFER
    // GL_SHADER
    // + GL_PROGRAM
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

auto GetDebugLabel(GLenum objectTypeKhr, GLenum objectTypeExt, GLuint objectId, char* outBuffer, size_t outBufferSize)
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

void LogDebugLabel(GLenum objectTypeKhr, GLenum objectTypeExt, GLuint objectId, char const* message) {
    constexpr size_t maxDebugLabelSize = 256U;
    static char debugLabel[maxDebugLabelSize];
    auto bytesWritten = GetDebugLabel(objectTypeKhr, objectTypeExt, objectId, debugLabel, maxDebugLabelSize);
    XLOG("{} (name={})", message, debugLabel);
}

} // namespace

namespace engine::gl {

void InitializeDebug() {
    if constexpr (!ENABLE_KHR_DEBUG_CALLBACK) { return; }
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLCALL(glEnable(GL_DEBUG_OUTPUT));
        void const* userParam = nullptr;
        GLCALL(glDebugMessageCallback(DebugOutputCallback, userParam));
        // NOTIFICATION severity is used together with debug groups
        if constexpr (!ENABLE_DEBUG_GROUP_LOGS) {
            auto disableType = [](GLenum type) {
                GLCALL(glDebugMessageControl(
                    /*source*/ GL_DONT_CARE, /*type*/ type, /*severity*/ GL_DEBUG_SEVERITY_NOTIFICATION,
                    /*count*/ 0, /*ids*/ nullptr, /*enabled*/ false));
            };
            disableType(GL_DEBUG_TYPE_PUSH_GROUP);
            disableType(GL_DEBUG_TYPE_POP_GROUP);
        }
    }
    if (GlExtensions::Supports(GlExtensions::ARB_debug_output)) {
        void const* userParam = nullptr;
        GLCALL(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB));
        GLCALL(glDebugMessageCallbackARB(DebugOutputCallback, userParam));
        // no NOTIFICATION severity in ARB extension
        // GLCALL(glDebugMessageControlARB(
        //     /*source*/ GL_DEBUG_SOURCE_OTHER_ARB,
        //     /*type*/ GL_DEBUG_TYPE_OTHER_ARB,
        //     /*severity*/ GL_DEBUG_SEVERITY_LOW_ARB,
        //     /*count*/ 0,
        //     /*ids*/ nullptr,
        //     /*enabled*/ false));
    }
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
    XLOGE("{}(0x{:x}) file: {}:{} line: {}", errLabel, err, fname, line, stmt);
    if (fatal) { std::terminate(); }
}

void PushDebugGroup(std::string_view label, GLuint userData) {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLCALL(glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, userData, label.size(), label.data()));
        return;
    }
    if (GlExtensions::Supports(GlExtensions::EXT_debug_marker)) {
        GLCALL(glPushGroupMarkerEXT(label.size(), label.data()));
        return;
    }
}

void PopDebugGroup() {
    assert(GlExtensions::IsInitialized());
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLCALL(glPopDebugGroup());
        return;
    }
    if (GlExtensions::Supports(GlExtensions::EXT_debug_marker)) {
        GLCALL(glPopGroupMarkerEXT());
        return;
    }
}

void DebugLabel(GpuBuffer const& buffer, std::string_view label) {
    ::DebugLabel(GL_BUFFER, GL_BUFFER_OBJECT_EXT, buffer.Id(), label);
}

auto GetDebugLabel(GpuBuffer const& buffer, char* outBuffer, size_t outBufferSize) -> size_t {
    return ::GetDebugLabel(GL_BUFFER, GL_BUFFER_OBJECT_EXT, buffer.Id(), outBuffer, outBufferSize);
}

void LogDebugLabel(GpuBuffer const& buffer, char const* message) {
    ::LogDebugLabel(GL_BUFFER, GL_BUFFER_OBJECT_EXT, buffer.Id(), message);
}

void DebugLabel(Vao const& vertexArray, std::string_view label) {
    ::DebugLabel(GL_VERTEX_ARRAY, GL_VERTEX_ARRAY_OBJECT_EXT, vertexArray.Id(), label);
}

auto GetDebugLabel(Vao const& vertexArray, char* outBuffer, size_t outBufferSize) -> size_t {
    return ::GetDebugLabel(GL_VERTEX_ARRAY, GL_VERTEX_ARRAY_OBJECT_EXT, vertexArray.Id(), outBuffer, outBufferSize);
}

void LogDebugLabel(Vao const& vao, char const* message) {
    ::LogDebugLabel(GL_VERTEX_ARRAY, GL_VERTEX_ARRAY_OBJECT_EXT, vao.Id(), message);
}

void DebugLabel(GpuProgram const& program, std::string_view label) {
    ::DebugLabel(GL_PROGRAM, GL_PROGRAM_OBJECT_EXT, program.Id(), label);
}

auto GetDebugLabel(GpuProgram const& program, char* outBuffer, size_t outBufferSize) -> size_t {
    return ::GetDebugLabel(GL_PROGRAM, GL_PROGRAM_OBJECT_EXT, program.Id(), outBuffer, outBufferSize);
}

void LogDebugLabel(GpuProgram const& program, char const* message) {
    ::LogDebugLabel(GL_PROGRAM, GL_PROGRAM_OBJECT_EXT, program.Id(), message);
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

void LogDebugLabel(void* glSyncObject, char const* message) {
    constexpr size_t maxDebugLabelSize = 256U;
    static char debugLabel[maxDebugLabelSize];
    auto bytesWritten = GetDebugLabel(glSyncObject, debugLabel, maxDebugLabelSize);
    XLOG("{} (name={})", message, debugLabel);
}

} // namespace engine::gl