#pragma once

#include "engine/Prelude.hpp"

namespace engine {

struct RenderCtx final {

public:
#define Self RenderCtx
    explicit Self()              = default;
    ~Self()                      = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    engine::i64 timeNs{0};
    engine::f32 timeSec{0.0f};
    engine::i64 prevTimeNs{0};
    engine::f32 prevFrametimeMs{0.0f};
    engine::f32 prevFPS{0.0f};

    void Update(engine::i64 currentTimeNs, RenderCtx& destination) const {
        destination.timeNs          = currentTimeNs;
        destination.timeSec         = static_cast<engine::f32>(currentTimeNs / 1000) * 0.000001;
        destination.prevTimeNs      = this->timeNs;
        engine::f32 frametimeMs     = static_cast<engine::f32>(currentTimeNs - this->timeNs) * 0.000001;
        destination.prevFrametimeMs = frametimeMs;
        destination.prevFPS         = 1000.0 / frametimeMs;
    }
};

} // namespace engine