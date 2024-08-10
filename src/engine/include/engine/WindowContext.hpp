#pragma once

#include "engine/Prelude.hpp"
#include <glm/vec2.hpp>
#include <functional>

namespace engine {

using ButtonCallback = std::function<void(bool, bool)>;
using AxisCallback   = std::function<void(float)>;

class WindowCtx final {

public:
#define Self WindowCtx
    explicit Self(GLFWwindow* window);
    ~Self()                      = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    auto IsInitialized [[nodiscard]] () const -> bool { return window_ != nullptr; }
    auto Window [[nodiscard]] () const -> GLFWwindow* { return window_; }
    auto WindowSize [[nodiscard]] () const -> glm::ivec2 { return windowSize_; }
    auto MousePosition [[nodiscard]] () const -> glm::vec2 { return mousePos_; }
    auto MouseInsideWindow [[nodiscard]] () const -> bool { return mouseInsideWindow_; }

    using GlfwKey         = int;
    using GlfwMouseButton = int;
    auto SetKeyboardCallback [[nodiscard]] (GlfwKey keyboardKey, ButtonCallback callback) -> ButtonCallback;
    auto SetMouseButtonCallback [[nodiscard]] (GlfwMouseButton mouseButton, ButtonCallback callback) -> ButtonCallback;

    void UpdateResolution(int64_t width, int64_t height);
    void UpdateCursorPosition(double xpos, double ypos);
    void UpdateCursorEntered(bool entered);

private:
    GLFWwindow* window_;
    glm::ivec2 windowSize_{0, 0};
    glm::vec2 mousePos_{0.0f, 0.0f};
    bool mouseInsideWindow_{false};

    std::unordered_map<GlfwKey, ButtonCallback> keys_{};
    std::unordered_map<GlfwKey, ButtonCallback> mouseButtons_{};

    friend void GlfwKeyCallback(GLFWwindow* window, GlfwKey key, int scancode, int action, int mods);
    friend void GlfwMouseButtonCallback(GLFWwindow* window, GlfwMouseButton button, int action, int mods);
};

} // namespace engine
