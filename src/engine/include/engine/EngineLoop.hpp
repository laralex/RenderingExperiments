#pragma once

#include "engine/Precompiled.hpp"
#include "engine/RenderContext.hpp"
#include "engine/WindowContext.hpp"

namespace engine {

struct EngineCtx;
using EngineHandle = EngineCtx*;

// NOTE: no CpuView is used here, because this wrapper is not suitable for void data
using RenderCallback = void (*)(RenderCtx const&, WindowCtx const&, void* userData);

auto CreateEngine [[nodiscard]] () -> std::optional<EngineHandle>;
void DestroyEngine(EngineHandle);

auto GetWindowContext [[nodiscard]] (EngineHandle) -> WindowCtx&;
auto SetRenderCallback [[nodiscard]] (EngineHandle, RenderCallback newCallback) -> RenderCallback;
void SetApplicationData(EngineHandle, void* applicationData);
void BlockOnLoop(EngineHandle);

enum class UserActionType : size_t {
    RENDER = 0,
    WINDOW,
    NUM_TYPES,
};
using UserAction = std::function<void(void* applicationData)>;
void QueueForNextFrame(EngineHandle, UserActionType type, UserAction action);

} // namespace engine
