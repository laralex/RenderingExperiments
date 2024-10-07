#pragma once

namespace engine::gl {

enum class RenderState {
    DEPTH_TEST_WRITE,
    DEPTH_TEST_READONLY,
    DEPTH_NOTEST_WRITE,
    DEPTH_NOTEST_READONLY,
    CULLING_BACK,
    CULLING_NONE,
};

class GlRenderStateRegistry final {

public:
#define Self GlRenderStateRegistry
    explicit Self() noexcept     = default;
    ~Self()                      = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    static void DepthTestWrite();
    static void DepthTest();
    static void DepthAlwaysWrite();
    static void DepthAlways();
    static void CullBack();
    static void CullNone();
    static void Apply(RenderState newState);
};

}