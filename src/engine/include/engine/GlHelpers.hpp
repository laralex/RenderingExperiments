#pragma once

#include "engine/Prelude.hpp"
#include "engine/GlBuffer.hpp"

#ifdef XDEBUG
// clang-format off
//#define GLCALL(stmt) do { stmt; engine::gl::CheckOpenGlError(#stmt, __FILE__, __LINE__, /* fatal */ false); } while(0)
#define GLCALL(stmt) stmt
// clang-format on
#else
#define GLCALL(stmt) stmt
#endif

namespace engine::gl {

void InitializeOpenGl();
void CheckOpenGlError(const char* stmt, const char* fname, int line, bool fatal);

class GpuBuffer;
void DebugLabel(GpuBuffer const& buffer, std::string_view label);
auto GetDebugLabel(GpuBuffer const& buffer, char* outBuffer, size_t outBufferSize) -> size_t;

class Vao;
void DebugLabel(Vao const& vertexArrayObject, std::string_view label);
auto GetDebugLabel[[nodiscard]](Vao const& vertexArrayObject, char* outBuffer, size_t outBufferSize) -> size_t;

void DebugLabel(void const* glSyncObject, std::string_view label);
auto GetDebugLabel[[nodiscard]](void* glSyncObject, char* outBuffer, size_t outBufferSize) -> size_t;

} // namespace engine::gl