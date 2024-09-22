#include "engine/EngineLoop.hpp"
#include "engine_private/Prelude.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <memory>
#include <queue>
#include <vector>

namespace {

auto InitializeCommonResources() -> bool;
void DestroyCommonResources();
auto CreateWindow [[nodiscard]] (int width, int height) -> GLFWwindow*;

} // namespace

namespace engine {

struct EnginePersistentData {
#define Self EnginePersistentData
    explicit Self() = default;
    ~Self() noexcept {
        if (!isInitialized) { return; }
        --numEngineInstances;
        XLOGW("EnginePersistentData dtor");
        // TODO: resources leaked
        // if (numEngineInstances == 0) { DestroyCommonResources(); }
    }
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    auto Initialize() -> EngineResult {
        if (EnginePersistentData::numEngineInstances == 0 && !InitializeCommonResources()) {
            return EngineResult::WINDOW_CREATION_ERROR;
        }
        GLFWwindow* window = CreateWindow(800, 600);
        if (window == nullptr) { return EngineResult::WINDOW_CREATION_ERROR; }

        windowCtx = WindowCtx{window};

        spdlog::set_pattern("[Δt=%8i us] [tid=%t] %^[%L]%$ %v");
        glfwSetWindowUserPointer(window, &windowCtx);

        isInitialized = true;
        frameIdx      = 1U;
        frameHistory.resize(256U);

        ++numEngineInstances;
        return EngineResult::SUCCESS;
    }
    WindowCtx windowCtx{nullptr};
    void* applicationData{nullptr}; // user-provided external data
    std::vector<RenderCtx> frameHistory{};
    int64_t frameIdx{1U};
    RenderCallback renderCallback{[](RenderCtx const&, WindowCtx const&, void*) {}};
    std::queue<UserAction> actionQueues[static_cast<size_t>(UserActionType::NUM_TYPES)]{};
    bool isInitialized = false;
    ENGINE_STATIC static int64_t numEngineInstances;
};

ENGINE_STATIC int64_t EnginePersistentData::numEngineInstances = 0;

struct EngineCtx {
#define Self EngineCtx
    explicit Self() = default;
    ~Self() noexcept { Dispose(); }
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self
    auto Initialize() -> EngineResult {
        persistent = std::make_shared<EnginePersistentData>();
        return persistent->Initialize();
    }
    void Dispose() {
        if (persistent) {
            XLOGW("EngineCtx dtor");
        }
        // persistent.reset();
    }

    std::shared_ptr<EnginePersistentData> persistent;
};

void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

} // namespace engine

namespace {

constexpr size_t MAX_ENGINES = 16U;

std::array<std::optional<engine::EngineCtx>, MAX_ENGINES> g_engines{};

// NOTE: can't use std::string_view (third party callback)
void GlfwErrorCallback(int errCode, char const* message) { XLOGE("GLFW_ERROR({}): {}", errCode, message); }

auto InitializeCommonResources() -> bool {
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) {
        XLOGE("Failed to initialize GLFW");
        return false;
    }
    return true;
}

void DestroyCommonResources() {
    XLOGW("Terminate GLFW");
    glfwTerminate();
}

void GlfwCursorEnterCallback(GLFWwindow* window, int entered) {
    auto ctx = static_cast<engine::WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    ctx->UpdateCursorEntered(entered);
}

void GlfwCursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    auto ctx = static_cast<engine::WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    ctx->UpdateCursorPosition(xpos, ypos);
}

void GlfwResizeCallback(GLFWwindow* window, int width, int height) {
    auto ctx = static_cast<engine::WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    ctx->UpdateResolution(width, height);
}

auto CreateWindow [[nodiscard]] (int width, int height) -> GLFWwindow* {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    int isDebugContext = GLFW_FALSE;
    if constexpr (engine::DEBUG_BUILD) {
        isDebugContext = GLFW_TRUE;
        XLOG("! GLFW - making debug-context for OpenGL");
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
        XLOGE("Failed to create GLFW window")
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))) {
        XLOGE("Failed to initialize GLAD");
        return nullptr;
    }

    glfwSetFramebufferSizeCallback(window, GlfwResizeCallback);
    glfwSetKeyCallback(window, engine::GlfwKeyCallback);
    glfwSetMouseButtonCallback(window, engine::GlfwMouseButtonCallback);
    glfwSetCursorEnterCallback(window, GlfwCursorEnterCallback);
    glfwSetCursorPosCallback(window, GlfwCursorPositionCallback);
    glfwSwapInterval(1);
    glfwSetTime(0.0);

    return window;
}

auto UpdateEngineLoop [[nodiscard]] (std::vector<engine::RenderCtx>& frameHistory, size_t frameIdx)
-> engine::RenderCtx& {
    using namespace engine;
    size_t const frameHistoryIdx     = frameIdx % frameHistory.size();
    size_t const prevFrameHistoryIdx = (frameIdx - 1) % frameHistory.size();

    RenderCtx& renderCtx           = frameHistory[frameHistoryIdx];
    RenderCtx const& prevRenderCtx = frameHistory[prevFrameHistoryIdx];
    prevRenderCtx.Update(glfwGetTimerValue(), frameIdx, renderCtx);

    if (frameHistoryIdx == 0) { XLOG("{} FPS, {} ms", renderCtx.prevFPS, renderCtx.prevFrametimeMs); }
    return renderCtx;
}

} // namespace

namespace engine {

std::queue<UserAction>& GetEngineQueue(EngineHandle engine, UserActionType type) {
    assert(engine != nullptr && engine->persistent.get() && "Invalid engine for GetEngineQueue");
    assert(
        static_cast<size_t>(type) < std::size(engine->persistent->actionQueues) && "Invalid queue for GetEngineQueue");
    return engine->persistent->actionQueues[static_cast<size_t>(type)];
}

void ExecuteQueue(EngineHandle engine, std::queue<UserAction>& queue) {
    auto* appData = engine->persistent->applicationData;
    while (!queue.empty()) {
        auto& action = queue.front();
        action(appData);
        queue.pop();
    }
}

void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    if (auto const found = ctx->keys_.find(key); found != ctx->keys_.end()) {
        found->second(action == GLFW_PRESS, action == GLFW_RELEASE, static_cast<WindowCtx::KeyModFlags>(mods));
    }
}

void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    ctx->UpdateMouseButton(button, action, mods);
}

ENGINE_EXPORT auto CreateEngine() -> EngineHandle {
    for (auto& engineSlot : g_engines) {
        if (engineSlot != std::nullopt) { continue; }
        engineSlot = std::optional{EngineCtx{}};
        return &*engineSlot;
    }
    return ENGINE_HANDLE_NULL;
}

ENGINE_EXPORT auto ColdStartEngine(EngineHandle engine) -> EngineResult {
    if (engine == ENGINE_HANDLE_NULL) { return EngineResult::ERROR_ENGINE_NULL; }

    return engine->Initialize();
}

ENGINE_EXPORT auto HotStartEngine(EngineHandle engine, std::shared_ptr<EnginePersistentData> data) -> EngineResult {
    if (engine == ENGINE_HANDLE_NULL) { return EngineResult::ERROR_ENGINE_NULL; }
    engine->persistent = data;
    return EngineResult::SUCCESS;
}

ENGINE_EXPORT auto DestroyEngine(EngineHandle engine) -> std::shared_ptr<EnginePersistentData> {
    if (engine == ENGINE_HANDLE_NULL) { return nullptr; }
    std::shared_ptr<EnginePersistentData> engineData{engine->persistent};

    for (auto& engineSlot : g_engines) {
        if (engineSlot == std::nullopt || &*engineSlot != engine) { continue; }
        engineSlot = std::nullopt;
    }

    return engineData;
}

ENGINE_EXPORT auto TickEngine(EngineHandle engine) -> EngineResult {
    if (engine == ENGINE_HANDLE_NULL) [[unlikely]] { return EngineResult::ERROR_ENGINE_NULL; }
    if (!engine->persistent) [[unlikely]] { return EngineResult::ERROR_ENGINE_NOT_INITIALIZED; }

    EnginePersistentData& engineData = *engine->persistent;
    WindowCtx& windowCtx             = engineData.windowCtx;
    GLFWwindow* window               = windowCtx.Window();
    if (glfwWindowShouldClose(window)) { return EngineResult::WINDOW_CLOSED_NORMALLY; }
    auto& windowQueue = GetEngineQueue(engine, UserActionType::WINDOW);
    auto& renderQueue = GetEngineQueue(engine, UserActionType::RENDER);
    ExecuteQueue(engine, windowQueue);
    ExecuteQueue(engine, renderQueue);

    RenderCtx& renderCtx = UpdateEngineLoop(engineData.frameHistory, engineData.frameIdx);
    engineData.renderCallback(renderCtx, windowCtx, engineData.applicationData);

    glfwSwapBuffers(window);
    glfwPollEvents();
    windowCtx.OnFrameEnd();

    ++engineData.frameIdx;
    return EngineResult::SUCCESS;
}

ENGINE_EXPORT auto GetWindowContext(EngineHandle engine) -> WindowCtx& {
    assert(engine != ENGINE_HANDLE_NULL && engine->persistent);
    return engine->persistent->windowCtx;
}

ENGINE_EXPORT auto SetRenderCallback(EngineHandle engine, RenderCallback newCallback) -> RenderCallback {
    if (engine == ENGINE_HANDLE_NULL) [[unlikely]] {
        // TODO: error reporting
        return nullptr;
    }
    if (!engine->persistent) [[unlikely]] { return nullptr; }
    auto oldCallback                   = engine->persistent->renderCallback;
    engine->persistent->renderCallback = newCallback;
    return oldCallback;
}

ENGINE_EXPORT void SetApplicationData(EngineHandle engine, void* applicationData) {
    if (engine == ENGINE_HANDLE_NULL) [[unlikely]] {
        // TODO: error reporting
        return;
    }
    if (!engine->persistent) [[unlikely]] { return; }
    engine->persistent->applicationData = applicationData;
}

ENGINE_EXPORT auto GetApplicationData(EngineHandle engine) -> void* {
    if (engine == ENGINE_HANDLE_NULL) [[unlikely]] {
        // TODO: error reporting
        return nullptr;
    }
    if (!engine->persistent) [[unlikely]] { return nullptr; }
    return engine->persistent->applicationData;
}

ENGINE_EXPORT void QueueForNextFrame(EngineHandle engine, UserActionType type, UserAction action) {
    if (engine == ENGINE_HANDLE_NULL) [[unlikely]] {
        XLOGE("Failed to run QueueForNextFrame, invalid engine given");
        return;
    }
    if (!engine->persistent.get()) [[unlikely]] {
        XLOGE("Failed to run QueueForNextFrame, engine data is uninitialized");
        return;
    }
    GetEngineQueue(engine, type).push(action);
}

} // namespace engine
