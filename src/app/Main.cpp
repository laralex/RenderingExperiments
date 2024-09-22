#include "app/App.hpp"

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

    if (auto result = cr_main(&crCtx, CR_LOAD); result != 0) { return result; }
    while (true) {
        if (auto result = cr_main(&crCtx, CR_STEP); result != 0) { break; }
    }
    if (auto result = cr_main(&crCtx, CR_UNLOAD); result != 0) { return result; }
    if (auto result = cr_main(&crCtx, CR_CLOSE); result != 0) { return result; }
    return 0;
}