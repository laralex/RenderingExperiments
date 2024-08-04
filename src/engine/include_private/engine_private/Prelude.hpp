#pragma once

#include <cassert>
#include <glad/gl.h>

#if defined(_MSC_VER)
//  Microsoft
#define ENGINE_EXPORT __declspec(dllexport)
#define ENGINE_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
//  GCC
#define ENGINE_EXPORT __attribute__((visibility("default")))
#define ENGINE_IMPORT
#else
//  do nothing and hope for the best?
#define ENGINE_EXPORT
#define ENGINE_IMPORT
#pragma warning Unknown dynamic link import / export semantics.
#endif

namespace engine::private_ {} // namespace engine::private_
