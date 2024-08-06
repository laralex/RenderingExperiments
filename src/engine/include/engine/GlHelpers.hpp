#pragma once

#include "engine/Prelude.hpp"
#include "engine/GlBuffer.hpp"

#ifdef XDEBUG
// clang-format off
//#define GLCALL(stmt) do { stmt; engine::gl::CheckOpenGLError(#stmt, __FILE__, __LINE__, /* fatal */ false); } while(0)
#define GLCALL(stmt) stmt
// clang-format on
#else
#define GLCALL(stmt) stmt
#endif

namespace engine::gl {

void InitializeGl();
void CheckOpenGLError(const char* stmt, const char* fname, int line, bool fatal);

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

class GpuBuffer;
void DebugLabel(GpuBuffer const& buffer, std::string_view label);
void GetDebugLabel(GpuBuffer const& buffer, char* outBuffer, size_t outBufferSize);

class Vao;
void DebugLabel(Vao const& vertexArrayObject, std::string_view label);
void GetDebugLabel(Vao const& vertexArrayObject, char* outBuffer, size_t outBufferSize);

void DebugLabel(void const* glSyncObject, std::string_view label);
void GetDebugLabel(void* glSyncObject, char* outBuffer, size_t outBufferSize);

} // namespace engine::gl