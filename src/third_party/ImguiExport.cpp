#if defined(__GNUC__)
#define IMGUI_API __attribute__((visibility("default"))) extern
#else
#define IMGUI_API extern
#endif
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>