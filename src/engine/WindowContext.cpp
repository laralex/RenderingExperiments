#include "engine/WindowContext.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using namespace engine;

WindowCtx::WindowCtx(GLFWwindow* window)
    : window_(window)
    , keys_()
    , mouseButtons_()
    , mousePos_{0, 0}
    , mouseInsideWindow_(false) {
    int w = 0, h = 0;
    if (window != nullptr) { glfwGetFramebufferSize(window, &w, &h); }
    windowSize_ = {w, h};
}

auto WindowCtx::SetKeyboardCallback(GlfwKey keyboardKey, ButtonCallback callback) -> ButtonCallback {
    auto found                 = keys_.find(keyboardKey);
    ButtonCallback oldCallback = nullptr;
    if (found != keys_.end()) { oldCallback = found->second; }
    keys_[keyboardKey] = callback;
    return oldCallback;
}

auto WindowCtx::SetMouseButtonCallback(GlfwMouseButton button, ButtonCallback callback) -> ButtonCallback {
    auto found                 = mouseButtons_.find(button);
    ButtonCallback oldCallback = nullptr;
    if (found != mouseButtons_.end()) { oldCallback = found->second; }
    mouseButtons_[button] = callback;
    return oldCallback;
}

void WindowCtx::UpdateResolution(engine::isize width, engine::isize height) {
    if (width < 0) { width = 0; }
    if (height < 0) { height = 0; }
    windowSize_ = {static_cast<engine::i32>(width), static_cast<engine::i32>(height)};
}

void WindowCtx::UpdateCursorPosition(engine::f64 xpos, engine::f64 ypos) {
    mousePos_ = {static_cast<engine::f32>(xpos), static_cast<engine::f32>(ypos)};
}

void WindowCtx::UpdateCursorEntered(bool entered) { mouseInsideWindow_ = entered; }