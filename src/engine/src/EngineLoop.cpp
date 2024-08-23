#include "engine/EngineLoop.hpp"
#include "engine_private/Prelude.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <queue>
#include <vector>

using namespace engine;

namespace engine {

struct EngineCtx {
#define Self EngineCtx
    explicit Self(WindowCtx&& windowCtx);
    ~Self() noexcept { Dispose(); }
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    void Dispose();

    WindowCtx windowCtx{nullptr};
    void* applicationData{nullptr}; // user-provided external data
    std::vector<RenderCtx> frameHistory{};
    int64_t frameIdx{1U};
    RenderCallback renderCallback{[](RenderCtx const&, WindowCtx const&, void*) {}};
    std::queue<UserAction> actionQueues[static_cast<size_t>(UserActionType::NUM_TYPES)]{};
    bool isInitialized = false;

    static int64_t numEngineInstances;
};

void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

} // namespace engine

namespace {

std::optional<engine::EngineCtx> g_engineCtx = std::nullopt;

void GlfwErrorCallback(int errCode, char const* message) { XLOGE("GLFW_ERROR({}): {}", errCode, message); }

bool InitializeCommonResources() {
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) {
        XLOGE("Failed to initialize GLFW", 0)
        return false;
    }
    return true;
}

void DestroyCommonResources() { glfwTerminate(); }

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
    ctx->UpdateResolution(width, height);
}

auto CreateWindow [[nodiscard]] (int width, int height) -> GLFWwindow* {
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

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (monitor != nullptr) {
        GLFWvidmode const* mode = glfwGetVideoMode(monitor);

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    }

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

auto UpdateEngineLoop [[nodiscard]] (std::vector<RenderCtx>& frameHistory, size_t frameIdx) -> RenderCtx& {
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

int64_t EngineCtx::numEngineInstances = 0;

EngineCtx::EngineCtx(WindowCtx&& windowCtx)
    : windowCtx(std::move(windowCtx))
    , isInitialized(true)
    , frameIdx(1U)
    , frameHistory(256U) {
    numEngineInstances += 1;
}

void EngineCtx::Dispose() {
    if (!isInitialized) return;

    windowCtx       = WindowCtx{nullptr};
    applicationData = nullptr;
    renderCallback  = nullptr;
    frameHistory.clear();
    frameIdx         = 0;
    size_t numQueues = static_cast<size_t>(UserActionType::NUM_TYPES);
    for (size_t i = 0; i < numQueues; ++i) {
        actionQueues[i] = {};
    }

    numEngineInstances -= 1;
}

std::queue<UserAction>& GetEngineQueue(EngineHandle engine, UserActionType type) {
    return engine->actionQueues[static_cast<size_t>(type)];
}

void ExecuteQueue(EngineHandle engine, std::queue<UserAction>& queue) {
    while (!queue.empty()) {
        auto& action = queue.front();
        action(engine->applicationData);
        queue.pop();
    }
}

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

ENGINE_EXPORT auto CreateEngine() -> std::optional<EngineHandle> {
    if (EngineCtx::numEngineInstances == 0 && !InitializeCommonResources()) { return std::nullopt; }

    GLFWwindow* window = CreateWindow(800, 600);
    if (window == nullptr) { return std::nullopt; }

    g_engineCtx = std::optional{EngineCtx{WindowCtx{window}}};
    return std::optional{&*g_engineCtx};
}

ENGINE_EXPORT auto GetWindowContext(EngineHandle engine) -> WindowCtx& { return engine->windowCtx; }

ENGINE_EXPORT void DestroyEngine(EngineHandle engine) {
    if (engine->numEngineInstances == 0) { DestroyCommonResources(); }
}

ENGINE_EXPORT auto SetRenderCallback(EngineHandle engine, RenderCallback newCallback) -> RenderCallback {
    auto oldCallback       = engine->renderCallback;
    engine->renderCallback = newCallback;
    return oldCallback;
}

ENGINE_EXPORT void SetApplicationData(EngineHandle engine, void* applicationData) {
    engine->applicationData = applicationData;
}

ENGINE_EXPORT void BlockOnLoop(EngineHandle engine) {
    spdlog::set_pattern("[Δt=%8i us] [tid=%t] %^[%L]%$ %v");
    WindowCtx& windowCtx = engine->windowCtx;
    GLFWwindow* window   = windowCtx.Window();
    glfwSetWindowUserPointer(window, &windowCtx);

    auto& windowQueue = GetEngineQueue(engine, UserActionType::WINDOW);
    auto& renderQueue = GetEngineQueue(engine, UserActionType::RENDER);

    while (!glfwWindowShouldClose(window)) {
        ExecuteQueue(engine, windowQueue);
        ExecuteQueue(engine, renderQueue);

        RenderCtx& renderCtx = UpdateEngineLoop(engine->frameHistory, engine->frameIdx);
        engine->renderCallback(renderCtx, engine->windowCtx, engine->applicationData);

        glfwSwapBuffers(window);
        glfwPollEvents();

        ++engine->frameIdx;
    }
}

ENGINE_EXPORT void QueueForNextFrame(EngineHandle engine, UserActionType type, UserAction action) {
    GetEngineQueue(engine, type).push(action);
}

} // namespace engine
