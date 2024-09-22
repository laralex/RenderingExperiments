#include "app/App.hpp"
#include "engine/EngineLoop.hpp"

static app::ApplicationState g_app;

auto main() -> int {
    // emulate context of hot reloading library CR
    cr_plugin crCtx{
        .p                    = nullptr,
        .userdata             = &g_app,
        .version              = 0,
        .failure              = CR_NONE,
        .next_version         = 1,
        .last_working_version = 0,
    };

    constexpr int SUCCESS = static_cast<int>(engine::EngineResult::SUCCESS);
    if (auto result = cr_main(&crCtx, CR_LOAD); result != SUCCESS) { return result; }
    while (true) {
        if (auto result = cr_main(&crCtx, CR_STEP); result != SUCCESS) { break; }
    }
    if (auto result = cr_main(&crCtx, CR_UNLOAD); result != SUCCESS) { return result; }
    if (auto result = cr_main(&crCtx, CR_CLOSE); result != SUCCESS) { return result; }
    return 0;
}