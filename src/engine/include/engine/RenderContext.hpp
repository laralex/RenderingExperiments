#pragma once

namespace engine {

struct RenderCtx final {

public:
#define Self RenderCtx
    explicit Self() noexcept     = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    int64_t frameIdx{0};
    int64_t timeNs{0};
    float timeSec{0.0f};
    int64_t prevTimeNs{0};
    float prevFrametimeMs{0.0f};
    float prevFPS{0.0f};

    void Update(int64_t currentTimeNs, int64_t frameIdx, RenderCtx& destination) const;
};

// TODO: remove inline, move to cpp
inline void RenderCtx::Update(int64_t currentTimeNs, int64_t frameIdx, RenderCtx& destination) const {
    destination.frameIdx        = frameIdx;
    destination.timeNs          = currentTimeNs;
    destination.timeSec         = static_cast<float>(currentTimeNs / 1000) * 0.000001;
    destination.prevTimeNs      = this->timeNs;
    auto frametimeMs            = static_cast<float>(currentTimeNs - this->timeNs) * 0.000001;
    destination.prevFrametimeMs = frametimeMs;
    destination.prevFPS         = 1000.0 / frametimeMs;
}

} // namespace engine