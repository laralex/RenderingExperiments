#pragma once

#include "engine/Precompiled.hpp"
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
    ERROR_ENGINE_NOT_INITIALIZED = 100,
    WINDOW_CREATION_ERROR        = 200,
    WINDOW_USAGE_ERROR           = 300,
};

// NOTE: no CpuView is used here, because this wrapper is not suitable for void data
using RenderCallback = void (*)(RenderCtx const&, WindowCtx const&, void* userData);

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

auto GetWindowContext [[nodiscard]] (EngineHandle) -> WindowCtx&;
auto SetRenderCallback [[nodiscard]] (EngineHandle, RenderCallback newCallback) -> RenderCallback;
void SetApplicationData(EngineHandle, void* applicationData);
auto GetApplicationData [[nodiscard]] (EngineHandle engine) -> void*;

enum class UserActionType : size_t {
    RENDER = 0,
    WINDOW,
    NUM_TYPES,
};
using UserAction = std::function<void(void* applicationData)>;
void QueueForNextFrame(EngineHandle, UserActionType type, UserAction action);

} // namespace engine
