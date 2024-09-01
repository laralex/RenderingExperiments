#pragma once

#include <glad/gl.h>
#include <string_view>

#ifdef XDEBUG
// NOTE: there are 2 options to catch GL errors:
// (1) extension GL_KHR_debug + manually call engine::gl::CheckOpenGlError, minimal runtime cost,
//     but harder to isolate a command that produces errors
// (2) use this wrapper macro for all GL commands, it probably has immense runtime cost,
//     but tells exactly which line is at fault

// option (1)
// #define GLCALL(stmt) stmt
// option (2)
#define GLCALL(stmt)                                                                                                   \
    do {                                                                                                               \
        stmt;                                                                                                          \
        engine::gl::CheckOpenGlError(#stmt, __FILE__, __LINE__, /* fatal */ true);                                     \
    } while (0)

#else
#define GLCALL(stmt) stmt
#endif

namespace engine
{
    template<typename T>
    struct CpuMemory;
} // namespace engine

namespace engine::gl {
void InitializeDebug();

// NOTE: can't use std::string_view (fails to work with GLCALL macro)
void CheckOpenGlError(char const* stmt, char const* fname, int line, bool fatal);

// Helper object, pushes debug group in ctor, pops it in dtor
class DebugGroupCtx final {
public:
#define Self DebugGroupCtx
    explicit Self(std::string_view label, GLuint userData = 0U);
    ~Self() noexcept;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self
};

void PushDebugGroup(std::string_view label, GLuint userData = 0U);
void PopDebugGroup();

enum class GlObjectType {
    BUFFER = 0,
    VAO,
    PROGRAM,
    TEXTURE,
    RENDERBUFFER,
    SAMPLER,
    FRAMEBUFFER,
    NUM_OBJECT_TYPES,
};

void DebugLabelUnsafe(GLuint object, GlObjectType objectType, std::string_view label);
void LogDebugLabelUnsafe(GLuint object, GlObjectType objectType, std::string_view message);
auto GetDebugLabelUnsafe [[nodiscard]] (GLuint object, GlObjectType objectType, CpuMemory<char> outBuffer)
-> size_t;

class GpuBuffer;
void DebugLabel(GpuBuffer const& buffer, std::string_view label);
auto GetDebugLabel [[nodiscard]] (GpuBuffer const& buffer, CpuMemory<char> outBuffer) -> size_t;
void LogDebugLabel(GpuBuffer const& buffer, std::string_view message);

class Vao;
void DebugLabel(Vao const& vertexArrayObject, std::string_view label);
auto GetDebugLabel [[nodiscard]] (Vao const& vertexArrayObject, CpuMemory<char> outBuffer) -> size_t;
void LogDebugLabel(Vao const& vao, std::string_view message);

class GpuProgram;
void DebugLabel(GpuProgram const& program, std::string_view label);
auto GetDebugLabel [[nodiscard]] (GpuProgram const& program, CpuMemory<char> outBuffer) -> size_t;
void LogDebugLabel(GpuProgram const& program, std::string_view message);

class Texture;
void DebugLabel(Texture const& texture, std::string_view label);
auto GetDebugLabel [[nodiscard]] (Texture const& texture, CpuMemory<char> outBuffer) -> size_t;
void LogDebugLabel(Texture const& texture, std::string_view message);

class Renderbuffer;
void DebugLabel(Renderbuffer const& renderbuffer, std::string_view label);
auto GetDebugLabel [[nodiscard]] (Renderbuffer const& renderbuffer, CpuMemory<char> outBuffer) -> size_t;
void LogDebugLabel(Renderbuffer const& renderbuffer, std::string_view message);

class GpuSampler;
void DebugLabel(GpuSampler const& sampler, std::string_view label);
auto GetDebugLabel [[nodiscard]] (GpuSampler const& sampler, CpuMemory<char> outBuffer) -> size_t;
void LogDebugLabel(GpuSampler const& sampler, std::string_view message);

class Framebuffer;
void DebugLabel(Framebuffer const& fb, std::string_view label);
auto GetDebugLabel [[nodiscard]] (Framebuffer const& fb, CpuMemory<char> outBuffer) -> size_t;
void LogDebugLabel(Framebuffer const& fb, std::string_view message);

void DebugLabel(void const* glSyncObject, std::string_view label);
auto GetDebugLabel [[nodiscard]] (void* glSyncObject, CpuMemory<char> outBuffer) -> size_t;
void LogDebugLabel(void const* glSyncObject, std::string_view message);

} // namespace engine::gl