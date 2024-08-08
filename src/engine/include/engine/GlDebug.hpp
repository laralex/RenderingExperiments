#pragma once

// #include "engine/Prelude.hpp"
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
        engine::gl::CheckOpenGlError(#stmt, __FILE__, __LINE__, /* fatal */ true);                                    \
    } while (0)

#else
#define GLCALL(stmt) stmt
#endif

namespace engine::gl {
void InitializeDebug();
void CheckOpenGlError(const char* stmt, const char* fname, int line, bool fatal);

void PushDebugGroup(std::string_view label, GLuint userData = 0U);
void PopDebugGroup();

class GpuBuffer;
void DebugLabel(GpuBuffer const& buffer, std::string_view label);
auto GetDebugLabel [[nodiscard]] (GpuBuffer const& buffer, char* outBuffer, size_t outBufferSize) -> size_t;
void LogDebugLabel(GpuBuffer const& buffer, char const* message);

class Vao;
void DebugLabel(Vao const& vertexArrayObject, std::string_view label);
auto GetDebugLabel [[nodiscard]] (Vao const& vertexArrayObject, char* outBuffer, size_t outBufferSize) -> size_t;
void LogDebugLabel(Vao const& vao, char const* message);

class GpuProgram;
void DebugLabel(GpuProgram const& program, std::string_view label);
auto GetDebugLabel [[nodiscard]] (GpuProgram const& program, char* outBuffer, size_t outBufferSize) -> size_t;
void LogDebugLabel(GpuProgram const& program, char const* message);

void DebugLabel(void const* glSyncObject, std::string_view label);
auto GetDebugLabel [[nodiscard]] (void* glSyncObject, char* outBuffer, size_t outBufferSize) -> size_t;
void LogDebugLabel(void const* glSyncObject, char const* message);

} // namespace engine::gl