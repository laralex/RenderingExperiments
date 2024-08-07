#include "engine/RenderLoop.hpp"
#include "engine_private/Prelude.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>

using namespace engine;

void engine::GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void engine::GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

namespace {

WindowCtx g_windowCtx{nullptr};
void* g_applicationData = nullptr; // user-provided external data
std::vector<RenderCtx> g_frameHistory(256);
size_t g_frameIdx               = 0;
RenderCallback g_renderCallback = [](RenderCtx const&, WindowCtx const&, void*) {};

void GlfwErrorCallback(int errCode, char const* message) { XLOGE("GLFW_ERROR({}): {}", errCode, message); }

void GlfwCursorEnterCallback(GLFWwindow* window, int entered) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    ctx->UpdateCursorEntered(entered);
}

void GlfwCursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    ctx->UpdateCursorPosition(xpos, ypos);
}

void GlfwResizeCallback(GLFWwindow* window, int width, int height) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    glViewport(0, 0, width, height);
    ctx->UpdateResolution(width, height);
}

auto CreateWindow [[nodiscard]] (int width, int height) -> GLFWwindow* {
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
    int isDebugContext = GLFW_FALSE;
    if constexpr (engine::DEBUG_BUILD) {
        isDebugContext = GLFW_TRUE;
        XLOG("! GLFW - making debug-context for OpenGL", 0);
    }
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, isDebugContext);

    GLFWmonitor* monitor    = glfwGetPrimaryMonitor();
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

auto UpdateRenderLoop [[nodiscard]] (std::vector<RenderCtx>& frameHistory, size_t frameIdx) -> RenderCtx& {
    size_t const frameHistoryIdx     = frameIdx % frameHistory.size();
    size_t const prevFrameHistoryIdx = (frameIdx - 1) % frameHistory.size();

    RenderCtx& renderCtx           = frameHistory[frameHistoryIdx];
    RenderCtx const& prevRenderCtx = frameHistory[prevFrameHistoryIdx];
    prevRenderCtx.Update(glfwGetTimerValue(), renderCtx);

    if (frameHistoryIdx == 0) { XLOG("{} FPS, {} ms", renderCtx.prevFPS, renderCtx.prevFrametimeMs); }
    return renderCtx;
}

} // namespace

namespace engine {

void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    if (auto const found = ctx->keys_.find(key); found != ctx->keys_.end()) {
        found->second(action == GLFW_PRESS, action == GLFW_RELEASE);
    }
}

void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    if (auto const found = ctx->mouseButtons_.find(button); found != ctx->mouseButtons_.end()) {
        found->second(action == GLFW_PRESS, action == GLFW_RELEASE);
    }
}

ENGINE_EXPORT auto Initialize() -> WindowCtx& {
    GLFWwindow* window = CreateWindow(800, 600);
    g_windowCtx        = WindowCtx{window};
    if (window == nullptr) {
        Terminate();
        return g_windowCtx;
    }
    return g_windowCtx;
}

ENGINE_EXPORT void Terminate() {
    glfwTerminate();
    g_windowCtx       = WindowCtx{nullptr};
    g_applicationData = nullptr;
    g_renderCallback  = nullptr;
}

ENGINE_EXPORT auto SetRenderCallback(RenderCallback newCallback) -> RenderCallback {
    auto oldCallback = g_renderCallback;
    g_renderCallback = newCallback;
    return oldCallback;
}

ENGINE_EXPORT void SetApplicationData(void* applicationData) { g_applicationData = applicationData; }

ENGINE_EXPORT void BlockOnGameLoop(WindowCtx& windowCtx) {
    GLFWwindow* window = windowCtx.Window();
    glfwSetWindowUserPointer(window, &windowCtx);

    while (!glfwWindowShouldClose(window)) {
        RenderCtx& renderCtx = UpdateRenderLoop(g_frameHistory, g_frameIdx);
        g_renderCallback(renderCtx, windowCtx, g_applicationData);

        glfwSwapBuffers(window);
        glfwPollEvents();

        ++g_frameIdx;
    }

    // NOTE: can't Terminate() by ourselves, because application has to free
    // their GL resources while the context is alive
}

} // namespace engine
