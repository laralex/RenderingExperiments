#include "engine/gl/Debug.hpp"
#include "engine/Precompiled.hpp"
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
    if constexpr(engine::DEBUG_BUILD) {
        // NVIDIA: Buffer detailed info: Buffer object 15 (bound to GL_ARRAY_BUFFER_ARB, usage hint is GL_STATIC_DRAW) will
        // use VIDEO memory as the source for buffer object operations.
        if (source == 0x8246U) { return; }
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
    }
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

void DebugLabel(
    GlContext const& gl, GLenum objectTypeKhr, GLenum objectTypeExt, GLuint objectId, std::string_view label) {
    using engine::gl::GlExtensions;
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
    if (gl.Extensions().Supports(GlExtensions::KHR_debug)) {
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
    if (gl.Extensions().Supports(GlExtensions::EXT_debug_label) && objectTypeExt != GL_NONE) {
        GLCALL(glLabelObjectEXT(objectTypeExt, objectId, label.size(), label.data()));
        return;
    }
}

auto GetDebugLabel(
    GlContext const& gl, GLenum objectTypeKhr, GLenum objectTypeExt, GLuint objectId, engine::CpuMemory<char> outBuffer)
    -> GLsizei {
    using engine::gl::GlExtensions;
    GLsizei bytesWritten = 0U;
    if (gl.Extensions().Supports(GlExtensions::KHR_debug)) {
        GLCALL(glGetObjectLabel(objectTypeKhr, objectId, outBuffer.NumBytes(), &bytesWritten, outBuffer.Begin()));
        return static_cast<size_t>(bytesWritten);
    }
    if (gl.Extensions().Supports(GlExtensions::EXT_debug_label) && objectTypeExt != GL_NONE) {
        GLCALL(glGetObjectLabelEXT(objectTypeExt, objectId, outBuffer.NumBytes(), &bytesWritten, outBuffer.Begin()));
        return static_cast<size_t>(bytesWritten);
    }
    return 0U;
}

void LogDebugLabel(
    GlContext const& gl, GLenum objectTypeKhr, GLenum objectTypeExt, GLuint objectId, std::string_view message) {
    constexpr size_t maxDebugLabelSize = 256U;
    static char debugLabel[maxDebugLabelSize];
    auto bytesWritten =
        GetDebugLabel(gl, objectTypeKhr, objectTypeExt, objectId, engine::CpuMemory{debugLabel, maxDebugLabelSize});
    XLOG("{} (id=0x{:08X}, name={})", message, objectId, debugLabel);
}

} // namespace anonymous

namespace engine::gl {

ENGINE_EXPORT void InitializeDebug(GlContext const& gl) {
    if constexpr (!ENABLE_KHR_DEBUG_CALLBACK) { return; }
    if (gl.Extensions().Supports(GlExtensions::KHR_debug)) {
        GLCALL(glEnable(GL_DEBUG_OUTPUT));
        CpuMemory<void> userParam{};
        GLCALL(glDebugMessageCallback(DebugOutputCallback, userParam.Begin()));
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
    if (gl.Extensions().Supports(GlExtensions::ARB_debug_output)) {
        CpuMemory<void> userParam{};
        GLCALL(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB));
        GLCALL(glDebugMessageCallbackARB(DebugOutputCallback, userParam.Begin()));
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

ENGINE_EXPORT void CheckOpenGlError(char const* stmt, char const* fname, int line, bool fatal) {
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) { return; }
    std::string_view errLabel = "UNKNOWN_ERROR";
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

ENGINE_EXPORT DebugGroupCtx::DebugGroupCtx(GlContext const& gl, std::string_view label, GLuint userData)
    : useCoreCommand_(gl.Extensions().Supports(GlExtensions::KHR_debug))
    , useExtensionCommand_(gl.Extensions().Supports(GlExtensions::EXT_debug_marker)) {
    PushDebugGroup(useCoreCommand_, useExtensionCommand_, label, userData);
}

ENGINE_EXPORT DebugGroupCtx::~DebugGroupCtx() noexcept { PopDebugGroup(useCoreCommand_, useExtensionCommand_); }

ENGINE_EXPORT void PushDebugGroup(GlContext const& gl, std::string_view label, GLuint userData) {
    PushDebugGroup(
        gl.Extensions().Supports(GlExtensions::KHR_debug), gl.Extensions().Supports(GlExtensions::EXT_debug_marker),
        label, userData);
}

ENGINE_EXPORT void PushDebugGroup(bool coreCmd, bool extensionCmd, std::string_view label, GLuint userData) {
    if (coreCmd) {
        GLCALL(glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, userData, label.size(), label.data()));
        return;
    }
    if (extensionCmd) {
        GLCALL(glPushGroupMarkerEXT(label.size(), label.data()));
        return;
    }
}

ENGINE_EXPORT void PopDebugGroup(GlContext const& gl) {
    PopDebugGroup(
        gl.Extensions().Supports(GlExtensions::KHR_debug), gl.Extensions().Supports(GlExtensions::EXT_debug_marker));
}

ENGINE_EXPORT void PopDebugGroup(bool coreCmd, bool extensionCmd) {
    if (coreCmd) {
        GLCALL(glPopDebugGroup());
        return;
    }
    if (extensionCmd) {
        GLCALL(glPopGroupMarkerEXT());
        return;
    }
}

ENGINE_EXPORT void DebugLabelUnsafe(GlContext const& gl, GLuint object, GlObjectType objectType, std::string_view label) {
    GLenum objectTypeKhr = GL_NONE;
    GLenum objectTypeExt = GL_NONE;
    FillDebugLabelEnums(objectType, objectTypeKhr, objectTypeExt);
    ::DebugLabel(gl, objectTypeKhr, objectTypeExt, object, label);
}

ENGINE_EXPORT void LogDebugLabelUnsafe(GlContext const& gl, GLuint object, GlObjectType objectType, std::string_view message) {
    GLenum objectTypeKhr = GL_NONE;
    GLenum objectTypeExt = GL_NONE;
    FillDebugLabelEnums(objectType, objectTypeKhr, objectTypeExt);
    ::LogDebugLabel(gl, objectTypeKhr, objectTypeExt, object, message);
}

ENGINE_EXPORT auto GetDebugLabelUnsafe(GlContext const& gl, GLuint object, GlObjectType objectType, engine::CpuMemory<char> outBuffer)
    -> size_t {
    GLenum objectTypeKhr = GL_NONE;
    GLenum objectTypeExt = GL_NONE;
    FillDebugLabelEnums(objectType, objectTypeKhr, objectTypeExt);
    return ::GetDebugLabel(gl, objectTypeKhr, objectTypeExt, object, outBuffer);
}

ENGINE_EXPORT void DebugLabel(GlContext const& gl, GpuBuffer const& buffer, std::string_view label) {
    ::DebugLabel(gl, GL_BUFFER, GL_BUFFER_OBJECT_EXT, buffer.Id(), label);
}

ENGINE_EXPORT auto GetDebugLabel(GlContext const& gl, GpuBuffer const& buffer, engine::CpuMemory<char> outBuffer) -> size_t {
    return ::GetDebugLabel(gl, GL_BUFFER, GL_BUFFER_OBJECT_EXT, buffer.Id(), outBuffer);
}

ENGINE_EXPORT void LogDebugLabel(GlContext const& gl, GpuBuffer const& buffer, std::string_view message) {
    ::LogDebugLabel(gl, GL_BUFFER, GL_BUFFER_OBJECT_EXT, buffer.Id(), message);
}

ENGINE_EXPORT void DebugLabel(GlContext const& gl, Vao const& vertexArray, std::string_view label) {
    ::DebugLabel(gl, GL_VERTEX_ARRAY, GL_VERTEX_ARRAY_OBJECT_EXT, vertexArray.Id(), label);
}

ENGINE_EXPORT auto GetDebugLabel(GlContext const& gl, Vao const& vertexArray, engine::CpuMemory<char> outBuffer) -> size_t {
    return ::GetDebugLabel(gl, GL_VERTEX_ARRAY, GL_VERTEX_ARRAY_OBJECT_EXT, vertexArray.Id(), outBuffer);
}

ENGINE_EXPORT void LogDebugLabel(GlContext const& gl, Vao const& vao, std::string_view message) {
    ::LogDebugLabel(gl, GL_VERTEX_ARRAY, GL_VERTEX_ARRAY_OBJECT_EXT, vao.Id(), message);
}

ENGINE_EXPORT void DebugLabel(GlContext const& gl, GpuProgram const& program, std::string_view label) {
    ::DebugLabel(gl, GL_PROGRAM, GL_PROGRAM_OBJECT_EXT, program.Id(), label);
}

ENGINE_EXPORT auto GetDebugLabel(GlContext const& gl, GpuProgram const& program, engine::CpuMemory<char> outBuffer) -> size_t {
    return ::GetDebugLabel(gl, GL_PROGRAM, GL_PROGRAM_OBJECT_EXT, program.Id(), outBuffer);
}

ENGINE_EXPORT void LogDebugLabel(GlContext const& gl, GpuProgram const& program, std::string_view message) {
    ::LogDebugLabel(gl, GL_PROGRAM, GL_PROGRAM_OBJECT_EXT, program.Id(), message);
}

ENGINE_EXPORT void DebugLabel(GlContext const& gl, Texture const& texture, std::string_view label) {
    ::DebugLabel(gl, GL_TEXTURE, GL_TEXTURE, texture.Id(), label);
}

ENGINE_EXPORT auto GetDebugLabel(GlContext const& gl, Texture const& texture, engine::CpuMemory<char> outBuffer) -> size_t {
    return ::GetDebugLabel(gl, GL_TEXTURE, GL_TEXTURE, texture.Id(), outBuffer);
}

ENGINE_EXPORT void LogDebugLabel(GlContext const& gl, Texture const& texture, std::string_view message) {
    ::LogDebugLabel(gl, GL_TEXTURE, GL_TEXTURE, texture.Id(), message);
}

ENGINE_EXPORT void DebugLabel(GlContext const& gl, Renderbuffer const& renderbuffer, std::string_view label) {
    ::DebugLabel(gl, GL_RENDERBUFFER, GL_RENDERBUFFER, renderbuffer.Id(), label);
}

ENGINE_EXPORT auto GetDebugLabel(GlContext const& gl, Renderbuffer const& renderbuffer, engine::CpuMemory<char> outBuffer) -> size_t {
    return ::GetDebugLabel(gl, GL_RENDERBUFFER, GL_RENDERBUFFER, renderbuffer.Id(), outBuffer);
}

ENGINE_EXPORT void LogDebugLabel(GlContext const& gl, Renderbuffer const& renderbuffer, std::string_view message) {
    ::LogDebugLabel(gl, GL_RENDERBUFFER, GL_RENDERBUFFER, renderbuffer.Id(), message);
}

ENGINE_EXPORT void DebugLabel(GlContext const& gl, GpuSampler const& sampler, std::string_view label) {
    ::DebugLabel(gl, GL_SAMPLER, GL_SAMPLER, sampler.Id(), label);
}

ENGINE_EXPORT auto GetDebugLabel(GlContext const& gl, GpuSampler const& sampler, engine::CpuMemory<char> outBuffer) -> size_t {
    return ::GetDebugLabel(gl, GL_SAMPLER, GL_SAMPLER, sampler.Id(), outBuffer);
}

ENGINE_EXPORT void LogDebugLabel(GlContext const& gl, GpuSampler const& sampler, std::string_view message) {
    ::LogDebugLabel(gl, GL_SAMPLER, GL_SAMPLER, sampler.Id(), message);
}

ENGINE_EXPORT void DebugLabel(GlContext const& gl, Framebuffer const& fb, std::string_view label) {
    ::DebugLabel(gl, GL_FRAMEBUFFER, GL_FRAMEBUFFER, fb.Id(), label);
}

ENGINE_EXPORT auto GetDebugLabel(GlContext const& gl, Framebuffer const& fb, engine::CpuMemory<char> outBuffer) -> size_t {
    return ::GetDebugLabel(gl, GL_FRAMEBUFFER, GL_FRAMEBUFFER, fb.Id(), outBuffer);
}

ENGINE_EXPORT void LogDebugLabel(GlContext const& gl, Framebuffer const& fb, std::string_view message) {
    ::LogDebugLabel(gl, GL_FRAMEBUFFER, GL_FRAMEBUFFER, fb.Id(), message);
}

ENGINE_EXPORT void DebugLabel(GlContext const& gl, void* glSyncObject, std::string_view label) {
    if (gl.Extensions().Supports(GlExtensions::KHR_debug)) {
        GLCALL(glObjectPtrLabel(glSyncObject, label.size(), label.data()));
    }
}

ENGINE_EXPORT auto GetDebugLabel(GlContext const& gl, void* glSyncObject, char* outBuffer, size_t outBufferSize) -> size_t {
    if (gl.Extensions().Supports(GlExtensions::KHR_debug)) {
        GLsizei bytesWritten = 0;
        GLCALL(glGetObjectPtrLabel(glSyncObject, outBufferSize, &bytesWritten, outBuffer));
        return static_cast<size_t>(bytesWritten);
    }
    return 0U;
}

ENGINE_EXPORT void LogDebugLabel(GlContext const& gl, void* glSyncObject, std::string_view message) {
    constexpr size_t maxDebugLabelSize = 256U;
    static char debugLabel[maxDebugLabelSize];
    auto bytesWritten = GetDebugLabel(gl, glSyncObject, debugLabel, maxDebugLabelSize);
    XLOG("{} (name={})", message, debugLabel);
}

} // namespace engine::gl