#include "engine/EngineLoop.hpp"

#define CR_HOST CR_UNSAFE // required in the host only and before including cr.h
#ifdef XDEBUG
#   define CR_DEBUG
#endif
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <cr.h>
#pragma clang diagnostic pop

#include <cassert>

using AppTerminationCallback = void (*)();
extern engine::RenderCallback g_externalRenderCallback;
extern AppTerminationCallback g_externalDestroyCallback;

static cr_plugin hotReloader{};

constexpr int32_t HOT_RELOAD_AFTER_FRAME = 60;
constexpr char const* HOT_RELOAD_LIBRARY = "libengine.so";

auto main(int argc, char *argv[]) -> int {

    assert(cr_plugin_open(hotReloader, HOT_RELOAD_LIBRARY)); // the full path to library

    g_externalRenderCallback = [](engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx, void* userData){
        bool const doReloadCheck = (windowCtx.frameIdx % HOT_RELOAD_AFTER_FRAME) == 0;
        auto const reloadResult = cr_plugin_update(hotReloader, doReloadCheck);
        switch (reloadResult) {
            case 0: break;
            case -1: XLOG("HotReload ERROR_UPDATE\n"); hotReloadCrashing = true; break;
            case -2: XLOG("HotReload ERROR_LOAD_UNLOAD=-2\n"); hotReloadCrashing = true; break;
            default: ("dei::cr::answer=%d\n", engineAnswer); engineClosing = true; break;
        }
    };
    g_externalRenderCallback = [&]() {
        cr_plugin_close(hotReloader);
    };
    return 0;
}