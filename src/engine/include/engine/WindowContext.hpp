#pragma once

#include "engine/Precompiled.hpp"
#include <functional>

namespace engine {

using ButtonCallback = std::function<void(bool, bool)>;
using AxisCallback   = std::function<void(float)>;

class WindowCtx final {

public:
#define Self WindowCtx
    explicit Self(GLFWwindow* window);
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    using GlfwKey         = int;
    using GlfwMouseButton = int;

    auto IsInitialized [[nodiscard]] () const -> bool { return window_ != nullptr; }
    auto Window [[nodiscard]] () const -> GLFWwindow* { return window_; }
    auto WindowSize [[nodiscard]] () const -> glm::ivec2 { return windowSize_; }

    auto MousePosition [[nodiscard]] () const -> glm::vec2 { return mousePos_; }
    auto MouseDelta [[nodiscard]] () const -> glm::vec2 { return mousePosDelta_; }
    auto IsMouseInsideWindow [[nodiscard]] () const -> bool { return mouseInsideWindow_; }
    auto IsMousePressed [[nodiscard]] (GlfwMouseButton mouseButton) const -> float;
    auto MousePressedState [[nodiscard]] () const -> glm::vec3 { return mousePress_; }

    auto SetKeyboardCallback [[nodiscard]] (GlfwKey keyboardKey, ButtonCallback callback) -> ButtonCallback;
    auto SetMouseButtonCallback [[nodiscard]] (GlfwMouseButton mouseButton, ButtonCallback callback) -> ButtonCallback;

    // TODO: maybe shouldn't be public, maybe make private + friend
    void UpdateResolution(int64_t width, int64_t height);
    void UpdateCursorPosition(double xpos, double ypos);
    void UpdateCursorEntered(bool entered);
    void UpdateMouseButton(GlfwMouseButton button, int action, int mods);
    void OnFrameEnd();

private:
    GLFWwindow* window_{nullptr};
    glm::ivec2 windowSize_{0, 0};
    glm::vec2 mousePosDelta_{0.0f, 0.0f};
    glm::vec2 mousePos_{0.0f, 0.0f};
    glm::vec2 mousePosPrev_{0.0f, 0.0f};
    glm::vec3 mousePress_{0.0f, 0.0f, 0.0f};
    float isNonFirstFrame_{0.0f};

    bool mouseInsideWindow_{false};

    std::unordered_map<GlfwKey, ButtonCallback> keys_{};
    std::unordered_map<GlfwKey, ButtonCallback> mouseButtons_{};

    friend void GlfwKeyCallback(GLFWwindow* window, GlfwKey key, int scancode, int action, int mods);
    // friend void GlfwMouseButtonCallback(GLFWwindow* window, GlfwMouseButton button, int action, int mods);
};

} // namespace engine
