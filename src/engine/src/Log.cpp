#include "engine/Log.hpp"
#include "engine/Precompiled.hpp"

namespace engine {

void InitLogging() {
    spdlog::set_pattern("[Δt=%8i us] [tid=%t] %^[%L]%$ %v");
    if constexpr (XDEBUG_BUILD) {
        if constexpr (XVERBOSITY >= 5) {
            spdlog::set_level(spdlog::level::debug);
        } else if constexpr (XVERBOSITY >= 4) {
            spdlog::set_level(spdlog::level::info);
        } else if constexpr (XVERBOSITY >= 3) {
            spdlog::set_level(spdlog::level::warn);
        } else if constexpr (XVERBOSITY >= 2) {
            spdlog::set_level(spdlog::level::err);
        } else if constexpr (XVERBOSITY >= 1) {
            spdlog::set_level(spdlog::level::critical);
        } else {
            spdlog::set_level(spdlog::level::off);
        }
    } else {
        spdlog::set_level(spdlog::level::warn);
    }
}

} // namespace engine