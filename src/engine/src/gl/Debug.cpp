#include "engine/Prelude.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Renderbuffer.hpp"
#include "engine/gl/Sampler.hpp"
#include "engine/gl/Texture.hpp"
#include "engine/gl/Vao.hpp"
#include "engine_private/Prelude.hpp"

namespace {

using namespace engine::gl;

constexpr bool ENABLE_KHR_DEBUG_CALLBACK = true;
constexpr bool ENABLE_DEBUG_GROUP_LOGS   = false;

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

void FillDebugLabelEnums(GlObjectType objectType, GLenum& objectTypeKhr, GLenum& objectTypeExt) {
    objectTypeKhr = GL_NONE;
    objectTypeExt = GL_NONE;
    switch (objectType) {
    case GlObjectType::BUFFER:
        objectTypeKhr = GL_BUFFER;
        objectTypeExt = GL_BUFFER_OBJECT_EXT;
        break;
    case GlObjectType::VAO:
        objectTypeKhr = GL_VERTEX_ARRAY;
        objectTypeExt = GL_VERTEX_ARRAY_OBJECT_EXT;
        break;
    case GlObjectType::PROGRAM:
        objectTypeKhr = GL_PROGRAM;
        objectTypeExt = GL_PROGRAM_OBJECT_EXT;
        break;
    case GlObjectType::TEXTURE:
        objectTypeKhr = objectTypeExt = GL_TEXTURE;
        break;
    case GlObjectType::RENDERBUFFER:
        objectTypeKhr = objectTypeExt = GL_RENDERBUFFER;
        break;
    case GlObjectType::SAMPLER:
        objectTypeKhr = objectTypeExt = GL_SAMPLER;
        break;
    case GlObjectType::FRAMEBUFFER:
        objectTypeKhr = objectTypeExt = GL_FRAMEBUFFER;
        break;
    default:
        std::terminate();
    }
}

void DebugLabel(GLenum objectTypeKhr, GLenum objectTypeExt, GLuint objectId, std::string_view label) {
    using engine::gl::GlExtensions;
    assert(GlExtensions::IsInitialized());
    // Debug labels support
    // + GL_BUFFER
    // + GL_PROGRAM
    // + GL_VERTEX_ARRAY
    // + GL_SAMPLER
    // + GL_TEXTURE
    // + GL_RENDERBUFFER
    // + GL_FRAMEBUFFER
    // + sync objects (pointers)
    // GL_SHADER
    // GL_QUERY
    // GL_PROGRAM_PIPELINE
    // GL_TRANSFORM_FEEDBACK
    if (GlExtensions::Supports(GlExtensions::KHR_debug)) {
        GLCALL(glObjectLabel(objectTypeKhr, objectId, label.size(), label.data()));
        return;
    }
    // + TEXTURE
    // + FRAMEBUFFER
    // + RENDERBUFFER,
    // + SAMPLER
    // + BUFFER_OBJECT_EXT
    // + PROGRAM_OBJECT_EXT
    // + VERTEX_ARRAY_OBJECT_EX
    // TRANSFORM_FEEDBACK
    // SHADER_OBJECT_EXT
    // QUERY_OBJECT_EXT
    // PROGRAM_PIPELINE_OBJECT_EX
    if (GlExtensions::Supports(GlExtensions::EXT_debug_label) && objectTypeExt != GL_NONE) {
        GLCALL(glLabelObjectEXT(objectTypeExt, objectId, label.size(), label.data()));
        return;
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

DebugGroupCtx::DebugGroupCtx(std::string_view label, GLuint userData) { PushDebugGroup(label, userData); }

DebugGroupCtx::~DebugGroupCtx() { PopDebugGroup(); }

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

void DebugLabelUnsafe(GLuint object, GlObjectType objectType, std::string_view label) {
    GLenum objectTypeKhr = GL_NONE;
    GLenum objectTypeExt = GL_NONE;
    FillDebugLabelEnums(objectType, objectTypeKhr, objectTypeExt);
    ::DebugLabel(objectTypeKhr, objectTypeExt, object, label);
}

void LogDebugLabelUnsafe(GLuint object, GlObjectType objectType, char const* message) {
    GLenum objectTypeKhr = GL_NONE;
    GLenum objectTypeExt = GL_NONE;
    FillDebugLabelEnums(objectType, objectTypeKhr, objectTypeExt);
    ::LogDebugLabel(objectTypeKhr, objectTypeExt, object, message);
}

auto GetDebugLabelUnsafe(GLuint object, GlObjectType objectType, char* outBuffer, size_t outBufferSize) -> size_t {
    GLenum objectTypeKhr = GL_NONE;
    GLenum objectTypeExt = GL_NONE;
    FillDebugLabelEnums(objectType, objectTypeKhr, objectTypeExt);
    return ::GetDebugLabel(objectTypeKhr, objectTypeExt, object, outBuffer, outBufferSize);
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

void DebugLabel(Texture const& texture, std::string_view label) {
    ::DebugLabel(GL_TEXTURE, GL_TEXTURE, texture.Id(), label);
}

auto GetDebugLabel(Texture const& texture, char* outBuffer, size_t outBufferSize) -> size_t {
    return ::GetDebugLabel(GL_TEXTURE, GL_TEXTURE, texture.Id(), outBuffer, outBufferSize);
}

void LogDebugLabel(Texture const& texture, char const* message) {
    ::LogDebugLabel(GL_TEXTURE, GL_TEXTURE, texture.Id(), message);
}

void DebugLabel(Renderbuffer const& renderbuffer, std::string_view label) {
    ::DebugLabel(GL_RENDERBUFFER, GL_RENDERBUFFER, renderbuffer.Id(), label);
}

auto GetDebugLabel(Renderbuffer const& renderbuffer, char* outBuffer, size_t outBufferSize) -> size_t {
    return ::GetDebugLabel(GL_RENDERBUFFER, GL_RENDERBUFFER, renderbuffer.Id(), outBuffer, outBufferSize);
}

void LogDebugLabel(Renderbuffer const& renderbuffer, char const* message) {
    ::LogDebugLabel(GL_RENDERBUFFER, GL_RENDERBUFFER, renderbuffer.Id(), message);
}

void DebugLabel(Sampler const& sampler, std::string_view label) {
    ::DebugLabel(GL_SAMPLER, GL_SAMPLER, sampler.Id(), label);
}

auto GetDebugLabel(Sampler const& sampler, char* outBuffer, size_t outBufferSize) -> size_t {
    return ::GetDebugLabel(GL_SAMPLER, GL_SAMPLER, sampler.Id(), outBuffer, outBufferSize);
}

void LogDebugLabel(Sampler const& sampler, char const* message) {
    ::LogDebugLabel(GL_SAMPLER, GL_SAMPLER, sampler.Id(), message);
}

void DebugLabel(Framebuffer const& fb, std::string_view label) {
    ::DebugLabel(GL_FRAMEBUFFER, GL_FRAMEBUFFER, fb.Id(), label);
}

auto GetDebugLabel(Framebuffer const& fb, char* outBuffer, size_t outBufferSize) -> size_t {
    return ::GetDebugLabel(GL_FRAMEBUFFER, GL_FRAMEBUFFER, fb.Id(), outBuffer, outBufferSize);
}

void LogDebugLabel(Framebuffer const& fb, char const* message) {
    ::LogDebugLabel(GL_FRAMEBUFFER, GL_FRAMEBUFFER, fb.Id(), message);
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