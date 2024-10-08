#include "engine/gl/GlRenderStateRegistry.hpp"
#include "engine/Precompiled.hpp"

namespace engine::gl {

void GlRenderStateRegistry::DepthTestWrite(GLenum depthCompare) {
    if (depthState_ == RenderState::DEPTH_TEST_WRITE && depthCompare_ == depthCompare) {
        return;
    }
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthFunc(depthCompare));
    GLCALL(glDepthMask(GL_TRUE));
    depthState_ = RenderState::DEPTH_TEST_WRITE;
    depthCompare_ = depthCompare;
}

void GlRenderStateRegistry::DepthTest(GLenum depthCompare) {
    if (depthState_ == RenderState::DEPTH_TEST_READONLY && depthCompare_ == depthCompare) {
        return;
    }
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthFunc(GL_LEQUAL));
    GLCALL(glDepthMask(GL_FALSE));
    depthState_ = RenderState::DEPTH_TEST_READONLY;
    depthCompare_ = depthCompare;
}

void GlRenderStateRegistry::DepthAlwaysWrite() {
    if (depthState_ == RenderState::DEPTH_NOTEST_WRITE) {
        return;
    }
    GLCALL(glDisable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_TRUE));
    depthState_ = RenderState::DEPTH_NOTEST_WRITE;
    depthCompare_ = GL_NONE;
}

void GlRenderStateRegistry::DepthAlways() {
    if (depthState_ == RenderState::DEPTH_NOTEST_READONLY) {
        return;
    }
    GLCALL(glDisable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_FALSE));
    depthState_ = RenderState::DEPTH_NOTEST_READONLY;
    depthCompare_ = GL_NONE;
}

void GlRenderStateRegistry::CullBack() {
    if (cullState_ == RenderState::CULLING_BACK) {
        return;
    }
    GLCALL(glEnable(GL_CULL_FACE));
    GLCALL(glCullFace(GL_BACK));
    cullState_ = RenderState::CULLING_BACK;
}

void GlRenderStateRegistry::CullNone() {
    if (cullState_ == RenderState::CULLING_NONE) {
        return;
    }
    GLCALL(glDisable(GL_CULL_FACE));
    cullState_ = RenderState::CULLING_NONE;
}

void GlRenderStateRegistry::DiscardCache() {
    depthState_ = RenderState::UNKNOWN;
    depthCompare_ = GL_NONE;
    cullState_ = RenderState::UNKNOWN;
}

void GlRenderStateRegistry::SetTo(RenderState newState) {
    switch (newState) {
        case RenderState::DEPTH_TEST_WRITE:
            return DepthTestWrite();
        case RenderState::DEPTH_TEST_READONLY:
            return DepthTest();
        case RenderState::DEPTH_NOTEST_WRITE:
            return DepthAlwaysWrite();
        case RenderState::DEPTH_NOTEST_READONLY:
            return DepthAlways();
        case RenderState::CULLING_BACK:
            return CullBack();
        case RenderState::CULLING_NONE:
            return CullNone();
        case RenderState::UNKNOWN:
            return DiscardCache();
    }
}

// TODO: string_view doesn't work for search when string is type of unordered_map key
void GlRenderStateRegistry::SetTo(char const* stateKey) {
    auto findHandle = name2setterHandle_.find(stateKey);
    if (findHandle == std::cend(name2setterHandle_)) {
        XLOGE("Unknown state key for GlRenderStateRegistry: {}", stateKey);
        return;
    }
    SetTo(findHandle->second);
}

void GlRenderStateRegistry::SetTo(RenderStateHandle RenderStateHandle) {
    auto findSetter = customStateSetters_.find(RenderStateHandle);
    if (findSetter == std::cend(customStateSetters_)) {
        XLOGE("No state registered in GlRenderStateRegistry: {}", RenderStateHandle);
        return;
    }
    auto const& setter = findSetter->second;
    setter();
    DiscardCache();
}

auto GlRenderStateRegistry::AddStateSetter(char const* stateKey, SetterFunc setter) -> RenderStateHandle {
    RenderStateHandle handle = nextFreeHandle_;
    name2setterHandle_[stateKey] = handle;
    customStateSetters_[handle] = setter;

    ++nextFreeHandle_;
    return handle;
}

auto GlRenderStateRegistry::FindStateSetterHandle(char const* stateKey) -> RenderStateHandle {
    auto findHandle = name2setterHandle_.find(stateKey);
    if (findHandle != std::cend(name2setterHandle_)) {
        return findHandle->second;
    }
    XLOGE("Unknown state key for GlRenderStateRegistry: {}", stateKey);
    return NULL_SETTER;
}

}