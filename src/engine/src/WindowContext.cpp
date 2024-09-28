#include "engine/WindowContext.hpp"

#include "engine_private/Prelude.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace engine {

ENGINE_EXPORT WindowCtx::WindowCtx(GLFWwindow* window)
    : window_(window) {
    int w = 0, h = 0;
    if (window != nullptr) { glfwGetFramebufferSize(window, &w, &h); }
    windowSize_ = {w, h};
}

ENGINE_EXPORT auto WindowCtx::SetKeyboardCallback(GlfwKey keyboardKey, ButtonCallback callback) -> ButtonCallback {
    auto found                 = keys_.find(keyboardKey);
    if (found == keys_.cend()) {
        keys_[keyboardKey] = callback;
        return nullptr;
    }
    ButtonCallback oldCallback = found->second;
    XLOGW("WindowCtx::SetKeyboardCallback overwriting key: {}", keyboardKey);
    found->second = callback;
    return oldCallback;
}

ENGINE_EXPORT auto WindowCtx::SetMouseButtonCallback(GlfwMouseButton button, ButtonCallback callback) -> ButtonCallback {
    auto found                 = mouseButtons_.find(button);
    if (found == mouseButtons_.cend()) {
        mouseButtons_[button] = callback;
        return nullptr;
    }
    ButtonCallback oldCallback = found->second;
    XLOGW("WindowCtx::SetMouseButtonCallback overwriting button: {}", button);
    found->second = callback;
    return oldCallback;
}

ENGINE_EXPORT void WindowCtx::UpdateResolution(int64_t width, int64_t height) {
    if (width < 0) { width = 0; }
    if (height < 0) { height = 0; }
    windowSize_ = {static_cast<int32_t>(width), static_cast<int32_t>(height)};
}

ENGINE_EXPORT void WindowCtx::UpdateCursorPosition(double xpos, double ypos) {
    glm::vec2 newPos{static_cast<float>(xpos), static_cast<float>(ypos)};
    mousePos_ = newPos;
}

ENGINE_EXPORT void WindowCtx::UpdateMouseButton(GlfwMouseButton mouseButton, int action, int mods) {
    // TODO: reduce overhead
    float state = static_cast<float>(action == GLFW_PRESS) - (action == GLFW_RELEASE);
    switch (mouseButton) {
    case GLFW_MOUSE_BUTTON_LEFT:
        mousePress_.x = state;
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        mousePress_.y = state;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        mousePress_.z = state;
        break;
    default:
        break;
    }
    if (auto const found = mouseButtons_.find(mouseButton); found != mouseButtons_.cend()) {
        found->second(action == GLFW_PRESS, action == GLFW_RELEASE, static_cast<WindowCtx::KeyModFlags>(mods));
    }
}

ENGINE_EXPORT void WindowCtx::UpdateKeyboardKey(GlfwKey keyboardKey, int action, int mods) {
    if (auto const found = keys_.find(keyboardKey); found != keys_.cend()) {
        found->second(action == GLFW_PRESS, action == GLFW_RELEASE, static_cast<WindowCtx::KeyModFlags>(mods));
    }
}

ENGINE_EXPORT auto WindowCtx::IsMousePressed(GlfwMouseButton mouseButton) const -> float {
    // TODO: reduce overhead
    switch (mouseButton) {
    case GLFW_MOUSE_BUTTON_LEFT:
        return mousePress_.x;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        return mousePress_.y;
    case GLFW_MOUSE_BUTTON_RIGHT:
        return mousePress_.z;
    default:
        return 0.0f;
    }
}

ENGINE_EXPORT void WindowCtx::UpdateCursorEntered(bool entered) { mouseInsideWindow_ = entered; }

ENGINE_EXPORT void WindowCtx::OnPollEvents() {
    mousePress_      = glm::max(glm::vec3{0.0f}, mousePress_);
    mousePosDelta_   = mousePos_ - isNonFirstFrame_ * mousePosPrev_;
    mousePosPrev_    = mousePos_;
    isNonFirstFrame_ = 1.0f;
}

ENGINE_EXPORT auto operator&(WindowCtx::KeyModFlags a, WindowCtx::KeyModFlags b) -> bool {
    using T = std::underlying_type_t<WindowCtx::KeyModFlags>;
    return static_cast<T>(a) & static_cast<T>(b);
}

} // namespace engine