#include <engine/GlHelpers.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <unordered_map>
#include <vector>

using ButtonCallback = std::function<void(bool, bool)>;
using AxisCallback = std::function<void(engine::f32)>;

// forward declaration required to preserve internal linkage
static void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

class WindowCtx final {

  public:
#define Self WindowCtx
    explicit Self(GLFWwindow* window);
    ~Self() = default;
    Self(Self const&) = delete;
    Self(Self&&) = default;
    void operator=(Self const&) = delete;
    void operator=(Self&&) = delete;
#undef Self

    using GlfwKey = int;
    using GlfwMouseButton = int;
    auto SetKeyboardCallback(GlfwKey keyboardKey, ButtonCallback callback) -> ButtonCallback;
    auto SetMouseButtonCallback(GlfwMouseButton mouseButton, ButtonCallback callback) -> ButtonCallback;
    void UpdateResolution(engine::isize width, engine::isize height);
    void UpdateCursorPosition(engine::f64 xpos, engine::f64 ypos);
    void UpdateCursorEntered(bool entered);
    auto WindowSize() const -> engine::ivec2 { return windowSize_; }
    auto MousePosition() const -> engine::vec2 { return mousePos_; }
    auto MouseInsideWindow() const -> bool { return mouseInsideWindow_; }
  private:
    engine::ivec2 windowSize_{0, 0};
    engine::vec2 mousePos_{0.0f, 0.0f};
    bool mouseInsideWindow_{false};

    std::unordered_map<GlfwKey, ButtonCallback> keys_{};
    std::unordered_map<GlfwKey, ButtonCallback> mouseButtons_{};

    friend void GlfwKeyCallback(GLFWwindow* window, GlfwKey key, int scancode, int action, int mods);
    friend void GlfwMouseButtonCallback(GLFWwindow* window, GlfwMouseButton button, int action, int mods);
};

WindowCtx::WindowCtx(GLFWwindow* window)
    : keys_(), mouseButtons_() {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    windowSize_ = {w, h};
}

auto WindowCtx::SetKeyboardCallback(GlfwKey keyboardKey, ButtonCallback callback) -> ButtonCallback {
    auto found = keys_.find(keyboardKey);
    ButtonCallback oldCallback = nullptr;
    if (found != keys_.end()) { oldCallback = found->second; }
    keys_[keyboardKey] = callback;
    return oldCallback;
}

auto WindowCtx::SetMouseButtonCallback(GlfwMouseButton button, ButtonCallback callback) -> ButtonCallback {
    auto found = mouseButtons_.find(button);
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

void WindowCtx::UpdateCursorEntered(bool entered) {
    mouseInsideWindow_ = entered;
}

struct RenderCtx final {
    engine::i64 timeNs{0};
    engine::f32 timeSec{0.0f};
    engine::i64 prevTimeNs{0};
    engine::f32 prevFrametimeMs{0.0f};
    engine::f32 prevFPS{0.0f};

    void Update(engine::i64 currentTimeNs, RenderCtx& destination) const {
        destination.timeNs = currentTimeNs;
        destination.timeSec = static_cast<engine::f32>(currentTimeNs / 1000) * 0.000001;
        destination.prevTimeNs = this->timeNs;
        engine::f32 frametimeMs = static_cast<engine::f32>(currentTimeNs - this->timeNs) * 0.000001;
        destination.prevFrametimeMs = frametimeMs;
        destination.prevFPS = 1000.0 / frametimeMs;
    }
};

static void GlfwErrorCallback(int errCode, char const* message) { XLOGE("GLFW_ERROR({}): {}", errCode, message); }

static void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    if (auto const found = ctx->keys_.find(key); found != ctx->keys_.end()) {
        found->second(action == GLFW_PRESS, action == GLFW_RELEASE);
    }
}

static void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    if (auto const found = ctx->mouseButtons_.find(button); found != ctx->mouseButtons_.end()) {
        found->second(action == GLFW_PRESS, action == GLFW_RELEASE);
    }
}

static void GlfwCursorEnterCallback(GLFWwindow* window, int entered) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    ctx->UpdateCursorEntered(entered);
}

static void GlfwCursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    ctx->UpdateCursorPosition(xpos, ypos);
}

static void GlfwResizeCallback(GLFWwindow* window, int width, int height) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    glViewport(0, 0, width, height);
    ctx->UpdateResolution(width, height);
}

static void Terminate() { glfwTerminate(); }

static void Render(RenderCtx const& ctx, WindowCtx const& windowCtx) {
    engine::f32 const red = 0.5f * (std::sin(ctx.timeSec) + 1.0f);
    glClearColor(red, 0.5f, 0.5f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

static auto CreateWindow(int width, int height) -> GLFWwindow* {
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) {
        XLOGE("Failed to initialize GLFW", 0)
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    GLFWvidmode const* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        XLOGE("Failed to create GLFW window", 0)
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))) {
        XLOGE("Failed to initialize GLAD", 0);
        return nullptr;
    }

    glfwSetFramebufferSizeCallback(window, GlfwResizeCallback);
    glfwSetKeyCallback(window, GlfwKeyCallback);
    glfwSetMouseButtonCallback(window, GlfwMouseButtonCallback);
    glfwSetCursorEnterCallback(window, GlfwCursorEnterCallback);
    glfwSetCursorPosCallback(window, GlfwCursorPositionCallback);
    glfwSwapInterval(1);
    glfwSetTime(0.0);

    return window;
}

static auto UpdateRenderLoop(std::vector<RenderCtx>& frameHistory, size_t frameIdx) -> RenderCtx& {
    size_t const frameHistoryIdx = frameIdx % frameHistory.size();
    size_t const prevFrameHistoryIdx = (frameIdx - 1) % frameHistory.size();

    RenderCtx& renderCtx = frameHistory[frameHistoryIdx];
    RenderCtx const& prevRenderCtx = frameHistory[prevFrameHistoryIdx];
    prevRenderCtx.Update(glfwGetTimerValue(), renderCtx);

    if (frameHistoryIdx == 0) { XLOG("{} FPS, {} ms", renderCtx.prevFPS, renderCtx.prevFrametimeMs); }
    return renderCtx;
}

auto main() -> int {
#ifdef XDEBUG
    std::cout << "! Compiled in DEBUG mode\n";
#endif

    GLFWwindow* window = CreateWindow(800, 600);
    if (window == nullptr) {
        Terminate();
        return -1;
    }

    WindowCtx windowCtx(window);
    glfwSetWindowUserPointer(window, &windowCtx);

    windowCtx.SetKeyboardCallback(
        GLFW_KEY_ESCAPE, [=](bool pressed, bool released) { glfwSetWindowShouldClose(window, true); });

    windowCtx.SetKeyboardCallback(GLFW_KEY_F, [=](bool pressed, bool released) {
        static bool setToFullscreen = true;
        if (!pressed) { return; }

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (setToFullscreen) {
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        } else {
            // TODO: avoid hardcoding resolution
            glfwSetWindowMonitor(window, nullptr, 0, 0, 800, 600, mode->refreshRate);
        }
        setToFullscreen = !setToFullscreen;
    });

    windowCtx.SetMouseButtonCallback(GLFW_MOUSE_BUTTON_LEFT, [&](bool pressed, bool released) {
        if (released) {
            auto mousePosition = windowCtx.MousePosition();
            XLOG("LMB {} pos: {},{}", windowCtx.MouseInsideWindow(), mousePosition.x, mousePosition.y);
        }
    });

    std::vector<RenderCtx> frameHistory(256);
    size_t frameIdx = 0;

    while (!glfwWindowShouldClose(window)) {
        RenderCtx& renderCtx = UpdateRenderLoop(frameHistory, frameIdx);
        Render(renderCtx, windowCtx);

        glfwSwapBuffers(window);
        glfwPollEvents();

        ++frameIdx;
    }

    XLOG("Window is closed gracefully", 0);
    Terminate();
    return 0;
}