#pragma once

#include "engine/Prelude.hpp"
#include "engine/RenderContext.hpp"
#include "engine/WindowContext.hpp"

namespace engine {

using RenderCallback = void (*)(RenderCtx const&, WindowCtx const&, void* userData);

auto Initialize [[nodiscard]] () -> WindowCtx&;
void Terminate();
auto SetRenderCallback [[nodiscard]] (RenderCallback newCallback) -> RenderCallback;
void SetApplicationData(void* applicationData);
void BlockOnGameLoop(WindowCtx& windowCtx);

} // namespace engine
