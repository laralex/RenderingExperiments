
#include "engine/Log.hpp"

#define CR_HOST CR_UNSAFE // required in the host only and before including cr.h
// #define CR_HOST CR_SAFEST

#ifdef XDEBUG
#define CR_DEBUG
#endif // XDEBUG

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <cr.h>
#pragma clang diagnostic pop

#include <cassert>

constexpr int32_t HOT_RELOAD_AFTER_FRAME = 50;

// must be full absolute path
constexpr char const* HOT_RELOAD_LIBRARY_PATH =
#ifdef XDEBUG
    "/home/alex/sidework/game-dev/learnopengl/build/debug/install/libapp.so";
#else
    "/home/alex/sidework/game-dev/learnopengl/build/release/install/libapp.so";
#endif

auto main(int argc, char* argv[]) -> int {
    cr_plugin hotReloader{};
    assert(cr_plugin_open(hotReloader, HOT_RELOAD_LIBRARY_PATH)); // the full path to library

    int64_t numUpdates = 0;
    while (true) {
        bool const doReloadCheck = (numUpdates++ % HOT_RELOAD_AFTER_FRAME) == 0;
        int updateResult         = cr_plugin_update(hotReloader, doReloadCheck);
        switch (updateResult) {
        case 0:
            break;
        case -1:
            XLOG("HotReload ERROR_UPDATE", 0);
            break;
        case -2:
            XLOG("HotReload ERROR_LOAD_UNLOAD=-2", 0);
            break;
        default:
            XLOG("HotReload app response={}", updateResult);
            break;
        }
        if (updateResult != 0) { break; }
    }
    cr_plugin_close(hotReloader);
    return 0;
}