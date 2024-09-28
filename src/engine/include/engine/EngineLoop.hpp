#pragma once

#include "engine/RenderContext.hpp"
#include "engine/WindowContext.hpp"

#include <memory>

namespace engine {

struct EngineCtx;
using EngineHandle                        = EngineCtx*;
constexpr EngineHandle ENGINE_HANDLE_NULL = nullptr;

struct EnginePersistentData;

enum class EngineResult : int32_t {
    SUCCESS                      = 0,
    WINDOW_CLOSED_NORMALLY       = 1,
    ERROR_ENGINE_NULL            = 100,
    ERROR_ENGINE_INIT            = 150,
    ERROR_ENGINE_NOT_INITIALIZED = 200,
    ERROR_WINDOW_INIT            = 1000,
    ERROR_WINDOW_USAGE           = 1100,
};

enum class KeyModFlags : int32_t {
    SHIFT    = 1 << 0,
    CONTROL  = 1 << 1,
    ALT      = 1 << 2,
    SUPER    = 1 << 3,
    CAPSLOCK = 1 << 4,
    NUMLOCK  = 1 << 5,
};

auto operator&(KeyModFlags a, KeyModFlags b) -> bool;

using ButtonCallback  = std::function<void(bool, bool, KeyModFlags)>;
using AxisCallback    = std::function<void(float)>;
using GlfwKey         = int;
using GlfwMouseButton = int;

using RenderCallback = void (*)(RenderCtx const&, WindowCtx const&, void* userData);

enum class UserActionType : size_t {
    RENDER = 0,
    WINDOW,
    NUM_TYPES,
};

struct UserAction {
    UserActionType type;
    std::function<void(void* applicationData)> callback;
    std::string label;
};

// Just create engine handle
auto CreateEngine [[nodiscard]] () -> EngineHandle;

// Initialize, allocate engine resources
auto ColdStartEngine [[nodiscard]] (EngineHandle) -> EngineResult;

// Initialize, reuse allocated engine resources
auto HotStartEngine [[nodiscard]] (EngineHandle, std::shared_ptr<EnginePersistentData>) -> EngineResult;

// Destroy engine handle (can't use anymore), returns engine resources, suitable for HotStartEngine
auto DestroyEngine [[nodiscard]] (EngineHandle) -> std::shared_ptr<EnginePersistentData>;

// Update inner state, provided render callback is also called
auto TickEngine [[nodiscard]] (EngineHandle) -> EngineResult;

// auto GetWindowContext [[nodiscard]] (EngineHandle) -> WindowCtx&;
auto SetRenderCallback [[nodiscard]] (EngineHandle, RenderCallback newCallback) -> RenderCallback;
auto SetKeyboardCallback [[nodiscard]] (GlfwKey keyboardKey, ButtonCallback callback) -> ButtonCallback;
auto SetMouseButtonCallback [[nodiscard]] (GlfwMouseButton mouseButton, ButtonCallback callback) -> ButtonCallback;

void SetApplicationData(EngineHandle, void* applicationData);
auto GetApplicationData [[nodiscard]] (EngineHandle engine) -> void*;

auto GetWindowContext [[nodiscard]] (EngineHandle engine) -> WindowCtx&;

void QueueForNextFrame(EngineHandle, UserAction&& action);

} // namespace engine