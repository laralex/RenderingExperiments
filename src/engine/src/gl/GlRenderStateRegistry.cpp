#include "engine/gl/GlRenderStateRegistry.hpp"
#include "engine/Precompiled.hpp"

namespace engine::gl {

void GlRenderStateRegistry::DepthTestWrite() {
    // if (state_ == RenderState::DEPTH_TEST_WRITE) {
    //     return;
    // }
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthFunc(GL_LEQUAL));
    GLCALL(glDepthMask(GL_TRUE));
    // state_ = RenderState::DEPTH_TEST_WRITE;
}

void GlRenderStateRegistry::DepthTest() {
    // if (state_ == RenderState::DEPTH_TEST_READONLY) {
    //     return;
    // }
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthFunc(GL_LEQUAL));
    GLCALL(glDepthMask(GL_FALSE));
    // state_ = RenderState::DEPTH_TEST_READONLY;
}

void GlRenderStateRegistry::DepthAlwaysWrite() {
    // if (state_ == RenderState::DEPTH_NOTEST_WRITE) {
    //     return;
    // }
    GLCALL(glDisable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_TRUE));
    // state_ = RenderState::DEPTH_NOTEST_WRITE;
}

void GlRenderStateRegistry::DepthAlways() {
    // if (state_ == RenderState::DEPTH_NOTEST_READONLY) {
    //     return;
    // }
    GLCALL(glDisable(GL_DEPTH_TEST));
    GLCALL(glDepthMask(GL_FALSE));
    // state_ = RenderState::DEPTH_NOTEST_READONLY;
}

void GlRenderStateRegistry::CullBack() {
    // if (state_ == RenderState::DEPTH_NOTEST_READONLY) {
    //     return;
    // }
    GLCALL(glEnable(GL_CULL_FACE));
    GLCALL(glCullFace(GL_BACK));
    // state_ = RenderState::DEPTH_NOTEST_READONLY;
}

void GlRenderStateRegistry::CullNone() {
    // if (state_ == RenderState::DEPTH_NOTEST_READONLY) {
    //     return;
    // }
    GLCALL(glDisable(GL_CULL_FACE));
    // state_ = RenderState::DEPTH_NOTEST_READONLY;
}

void GlRenderStateRegistry::Apply(RenderState newState) {
    switch (newState) {
        case RenderState::DEPTH_TEST_WRITE:
            return DepthTestWrite();
        case RenderState::DEPTH_TEST_READONLY:
            return DepthTest();
        case RenderState::DEPTH_NOTEST_WRITE:
            return DepthAlwaysWrite();
        case RenderState::DEPTH_NOTEST_READONLY:
            return DepthAlways();
        default:
            return;
    }
}

}