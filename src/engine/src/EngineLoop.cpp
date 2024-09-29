#include "engine/EngineLoop.hpp"

#include "engine/Precompiled.hpp"
#include "engine_private/Prelude.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <memory>
#include <queue>
#include <vector>

namespace engine {

struct EnginePersistentData {
#define Self EnginePersistentData
    explicit Self()              = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    WindowCtx windowCtx{nullptr};
    void* applicationData{nullptr}; // user-provided external data
    std::vector<RenderCtx> frameHistory{};
    int64_t frameIdx{1U};
    RenderCallback renderCallback{[](RenderCtx const&, WindowCtx const&, void*) {}};

    moodycamel::ConcurrentQueue<UserAction> actionQueues[static_cast<size_t>(UserActionType::NUM_TYPES)];
    bool isInitialized = false;
};

struct EngineCtx {
#define Self EngineCtx
    explicit Self()              = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    std::shared_ptr<EnginePersistentData> persistent;
};

} // namespace engine

namespace {

using ActionQueue            = moodycamel::ConcurrentQueue<engine::UserAction>;
constexpr size_t MAX_ENGINES = 16U;

ENGINE_STATIC int64_t g_numEngineInstances = 0;
ENGINE_STATIC std::array<std::optional<engine::EngineCtx>, MAX_ENGINES> g_engines{};

// NOTE: can't use std::string_view (third party callback)
void GlfwErrorCallback(int errCode, char const* message) { XLOGE("GLFW_ERROR({}): {}", errCode, message); }

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

void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto ctx = static_cast<engine::WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    ctx->UpdateMouseButton(button, action, mods);
}

void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    XLOGW("GLFW key {} scancode {} action {} mods {}", key, scancode, action, mods);
    auto ctx = static_cast<engine::WindowCtx*>(glfwGetWindowUserPointer(window));
    if (ctx == nullptr) { return; }
    ctx->UpdateKeyboardKey(key, action, mods);
}

void InitializeWindow(GLFWwindow* window) {
    glfwSetFramebufferSizeCallback(window, GlfwResizeCallback);
    glfwSetKeyCallback(window, GlfwKeyCallback);
    glfwSetMouseButtonCallback(window, GlfwMouseButtonCallback);
    glfwSetCursorEnterCallback(window, GlfwCursorEnterCallback);
    glfwSetCursorPosCallback(window, GlfwCursorPositionCallback);
    glfwSwapInterval(1);
    glfwSetTime(0.0);
}

auto CreateWindow [[nodiscard]] (int width, int height, std::string_view name, GLFWwindow* oldWindow = nullptr)
-> GLFWwindow* {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    int isDebugContext = GLFW_FALSE;
    if constexpr (engine::XDEBUG_BUILD) {
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

    GLFWwindow* window = glfwCreateWindow(width, height, name.data(), nullptr, oldWindow);
    if (window == nullptr) {
        XLOGE("Failed to create GLFW window")
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))) {
        XLOGE("Failed to initialize GLAD");
        return nullptr;
    }

    return window;
}

auto InitializeEngineData [[nodiscard]] (engine::EnginePersistentData& out) -> engine::EngineResult {
    XLOGW("InitializeEngineData");
    glfwSetErrorCallback(GlfwErrorCallback);

    if (!out.windowCtx.IsInitialized()) {
        GLFWwindow* window = CreateWindow(800, 600, "LearnOpenGL", nullptr);
        if (window == nullptr) { return engine::EngineResult::ERROR_WINDOW_INIT; }
        out.windowCtx = engine::WindowCtx{window};
        glfwSetWindowUserPointer(window, &out.windowCtx);
    }

    InitializeWindow(out.windowCtx.Window());

    out.isInitialized = true;
    out.frameIdx      = 1U;
    out.frameHistory.clear();
    out.frameHistory.resize(256U);

    return engine::EngineResult::SUCCESS;
}

auto UpdateEngineLoop [[nodiscard]] (std::vector<engine::RenderCtx>& frameHistory, size_t frameIdx)
-> engine::RenderCtx& {
    using namespace engine;
    size_t const frameHistoryIdx     = frameIdx % frameHistory.size();
    size_t const prevFrameHistoryIdx = (frameIdx - 1) % frameHistory.size();

    RenderCtx& renderCtx           = frameHistory[frameHistoryIdx];
    RenderCtx const& prevRenderCtx = frameHistory[prevFrameHistoryIdx];
    prevRenderCtx.Update(glfwGetTimerValue(), frameIdx, renderCtx);

    return renderCtx;
}

auto GetEngineQueue [[nodiscard]] (engine::EngineHandle engine, engine::UserActionType type) -> ActionQueue& {
    assert(engine != nullptr && engine->persistent.get() && "Invalid engine for GetEngineQueue");
    assert(
        static_cast<size_t>(type) < std::size(engine->persistent->actionQueues) && "Invalid queue for GetEngineQueue");
    return engine->persistent->actionQueues[static_cast<size_t>(type)];
}

void ExecuteQueue(engine::EngineHandle engine, ActionQueue& queue) {
    auto* appData = engine->persistent->applicationData;
    engine::UserAction destAction;
    while (queue.try_dequeue(destAction)) {
        destAction.callback(appData);
        if (std::size(destAction.label) > 0) {
            XLOGD("ExecuteQueue done: {}", destAction.label);
        }
    }
}

auto InitializeCommonResources [[nodiscard]] () -> bool {
    engine::InitLogging();
    XLOGW("glfwInit()");
    if (!glfwInit()) {
        XLOGE("Failed to initialize GLFW");
        return false;
    }
    return true;
}

auto DestroyCommonResources [[nodiscard]] () -> bool {
    XLOGW("glfwTerminate()");
    glfwTerminate();
    return true;
}

} // namespace

namespace engine {

ENGINE_EXPORT auto CreateEngine() -> engine::EngineHandle {
    for (auto& engineSlot : g_engines) {
        if (engineSlot != std::nullopt) { continue; }
        engineSlot = std::optional{engine::EngineCtx{}};
        ++g_numEngineInstances;
        return &*engineSlot;
    }
    return engine::ENGINE_HANDLE_NULL;
}

ENGINE_EXPORT auto ColdStartEngine(engine::EngineHandle engine) -> engine::EngineResult {
    if (engine == engine::ENGINE_HANDLE_NULL) { return engine::EngineResult::ERROR_ENGINE_NULL; }

    if (g_numEngineInstances == 1) {
        // first engine ever
        assert(InitializeCommonResources());
    }
    engine->persistent = std::make_shared<engine::EnginePersistentData>();
    return InitializeEngineData(*engine->persistent);
}

ENGINE_EXPORT auto HotStartEngine(engine::EngineHandle engine, std::shared_ptr<engine::EnginePersistentData> data)
    -> engine::EngineResult {
    if (engine == engine::ENGINE_HANDLE_NULL) { return engine::EngineResult::ERROR_ENGINE_NULL; }
    engine->persistent = data;
    // NOTE: I suppose, resettings the callbacks is not necessary for hot start
    // cr.h library loads the dynamic libraries to the same addresses
    return engine::EngineResult::SUCCESS;
}

ENGINE_EXPORT auto DestroyEngine(engine::EngineHandle engine) -> std::shared_ptr<engine::EnginePersistentData> {
    if (engine == engine::ENGINE_HANDLE_NULL) { return nullptr; }
    std::shared_ptr<engine::EnginePersistentData> engineData{engine->persistent};

    for (auto& engineSlot : g_engines) {
        if (engineSlot == std::nullopt || &*engineSlot != engine) { continue; }
        --g_numEngineInstances;
        engineSlot = std::nullopt;
    }

    // TODO: possible leaking resources
    // if (g_numEngineInstances == 0) {
    //     assert(DestroyCommonResources());
    // }

    return engineData;
}

ENGINE_EXPORT auto TickEngine(engine::EngineHandle engine) -> engine::EngineResult {
    using namespace engine;

    if (engine == ENGINE_HANDLE_NULL) [[unlikely]] { return EngineResult::ERROR_ENGINE_NULL; }
    if (!engine->persistent) [[unlikely]] { return EngineResult::ERROR_ENGINE_NOT_INITIALIZED; }

    EnginePersistentData& engineData = *engine->persistent;
    WindowCtx& windowCtx             = engineData.windowCtx;
    GLFWwindow* window               = windowCtx.Window();

    glfwPollEvents();
    windowCtx.OnPollEvents();

    if (glfwWindowShouldClose(window)) { return EngineResult::WINDOW_CLOSED_NORMALLY; }
    auto& windowQueue = GetEngineQueue(engine, UserActionType::WINDOW);
    auto& renderQueue = GetEngineQueue(engine, UserActionType::RENDER);
    ExecuteQueue(engine, windowQueue);
    ExecuteQueue(engine, renderQueue);

    RenderCtx& renderCtx = UpdateEngineLoop(engineData.frameHistory, engineData.frameIdx);
    engineData.renderCallback(renderCtx, windowCtx, engineData.applicationData);

    glfwSwapBuffers(window);

    ++engineData.frameIdx;
    return EngineResult::SUCCESS;
}

ENGINE_EXPORT auto GetWindowContext(engine::EngineHandle engine) -> engine::WindowCtx& {
    assert(engine != engine::ENGINE_HANDLE_NULL && engine->persistent && "GetWindowContext");
    return engine->persistent->windowCtx;
}

ENGINE_EXPORT auto SetRenderCallback(engine::EngineHandle engine, engine::RenderCallback newCallback)
    -> engine::RenderCallback {
    if (engine == engine::ENGINE_HANDLE_NULL) [[unlikely]] {
        // TODO: error reporting
        return nullptr;
    }
    if (!engine->persistent) [[unlikely]] { return nullptr; }
    XLOGW("EngineLoop::SetRenderCallback");
    auto oldCallback                   = engine->persistent->renderCallback;
    engine->persistent->renderCallback = newCallback;
    return oldCallback;
}

ENGINE_EXPORT void SetApplicationData(engine::EngineHandle engine, void* applicationData) {
    if (engine == engine::ENGINE_HANDLE_NULL) [[unlikely]] {
        // TODO: error reporting
        return;
    }
    if (!engine->persistent) [[unlikely]] { return; }
    engine->persistent->applicationData = applicationData;
}

ENGINE_EXPORT auto GetApplicationData(engine::EngineHandle engine) -> void* {
    if (engine == engine::ENGINE_HANDLE_NULL) [[unlikely]] {
        // TODO: error reporting
        return nullptr;
    }
    if (!engine->persistent) [[unlikely]] { return nullptr; }
    return engine->persistent->applicationData;
}

// NOTE: action must live until the task is dequeued
// No callback is provided on that, so just keep it alive for atleast a couple of frames
ENGINE_EXPORT void QueueForNextFrame(engine::EngineHandle engine, engine::UserAction&& action) {
    if (engine == engine::ENGINE_HANDLE_NULL) [[unlikely]] {
        XLOGE("Failed to run QueueForNextFrame, invalid engine given: {}", action.label);
        return;
    }
    if (!engine->persistent.get()) [[unlikely]] {
        XLOGE("Failed to run QueueForNextFrame, engine data is uninitialized: {}", action.label);
        return;
    }
    // NOTE: storing callbacks is dangerous for hot-reloading
    GetEngineQueue(engine, action.type).enqueue(std::move(action));
}

} // namespace engine