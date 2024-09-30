#pragma once

#include "engine/Precompiled.hpp"
#include <functional>

namespace engine {

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

    enum class KeyModFlags : int32_t {
        SHIFT    = 1 << 0,
        CONTROL  = 1 << 1,
        ALT      = 1 << 2,
        SUPER    = 1 << 3,
        CAPSLOCK = 1 << 4,
        NUMLOCK  = 1 << 5,
    };

    using ButtonCallback  = std::function<void(bool, bool, KeyModFlags)>;
    using AxisCallback    = std::function<void(float)>;
    using GlfwKey         = int;
    using GlfwMouseButton = int;

    auto IsInitialized [[nodiscard]] () const -> bool { return window_ != nullptr; }
    auto Window [[nodiscard]] () const -> GLFWwindow* { return window_.get(); }
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
    void UpdateKeyboardKey(GlfwKey keyboardKey, int action, int mods);
    void OnPollEvents();

private:
    // owns the window, destroys in dtor
    struct GlfwWindowDeleter { void operator()(GLFWwindow* w); };
    std::unique_ptr<GLFWwindow, GlfwWindowDeleter> window_{nullptr};
    glm::ivec2 windowSize_{0, 0};
    glm::vec2 mousePosDelta_{0.0f, 0.0f};
    glm::vec2 mousePos_{0.0f, 0.0f};
    glm::vec2 mousePosPrev_{0.0f, 0.0f};
    glm::vec3 mousePress_{0.0f, 0.0f, 0.0f};
    float isNonFirstFrame_{0.0f};

    bool mouseInsideWindow_{false};

    std::unordered_map<GlfwKey, ButtonCallback> keys_{};
    std::unordered_map<GlfwKey, ButtonCallback> mouseButtons_{};
};

auto operator&(WindowCtx::KeyModFlags a, WindowCtx::KeyModFlags b) -> bool;

} // namespace engine
