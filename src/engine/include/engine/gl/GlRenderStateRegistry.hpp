#pragma once

#include <glad/gl.h>
#include <unordered_map>
#include <string>
#include <functional>

namespace engine::gl {

enum class RenderState {
    UNKNOWN,
    DEPTH_TEST_WRITE,
    DEPTH_TEST_READONLY,
    DEPTH_NOTEST_WRITE,
    DEPTH_NOTEST_READONLY,
    CULLING_BACK,
    CULLING_NONE,
};

using RenderStateHandle = int32_t;

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


    using SetterFunc = void(*)();
    constexpr static RenderStateHandle NULL_SETTER = 0;

    void DepthTestWrite(GLenum depthCompare = GL_LEQUAL);
    void DepthTest(GLenum depthCompare = GL_LEQUAL);
    void DepthAlwaysWrite();
    void DepthAlways();
    void CullBack();
    void CullNone();
    void DiscardCache();
    void SetTo(RenderState newState);
    void SetTo(char const* newStateKey);
    void SetTo(RenderStateHandle newStateKey);

    auto AddStateSetter[[nodiscard]](char const* stateKey, SetterFunc setter) -> RenderStateHandle;
    auto FindStateSetterHandle[[nodiscard]](char const* stateKey) -> RenderStateHandle;

private:
    RenderState depthState_;
    GLenum depthCompare_;
    RenderState cullState_;
    std::unordered_map<RenderStateHandle, std::function<void()>> customStateSetters_;
    std::unordered_map<std::string, RenderStateHandle> name2setterHandle_;
    RenderStateHandle nextFreeHandle_ = 1;
};

}