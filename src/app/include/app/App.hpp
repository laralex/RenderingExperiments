#pragma once

#include "engine/Assets.hpp"
#include "engine/FirstPersonLocomotion.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/CommonRenderers.hpp"
#include "engine/gl/Context.hpp"
#include "engine/gl/GpuMesh.hpp"
#include "engine/gl/FlatRenderer.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Renderbuffer.hpp"
#include "engine/gl/SamplersCache.hpp"
#include "engine/gl/Texture.hpp"
#include "engine/gl/TextureUnits.hpp"
#include "engine/gl/ProgramOwner.hpp"
#include "engine/platform/FileChangeNotifier.hpp"
#include "engine/EngineLoop.hpp"
#include "engine/platform/IFileWatcher.hpp"
#include "engine/platform/posix/FileChangeNotifier.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace app {

struct UboDataSamplerTiling {
    GLint albedoIdx   = 0;
    GLint normalIdx   = 0;
    GLint specularIdx = 0;
    GLint __pad0      = 0;
    glm::vec4 uvScaleOffsets[256U];
};

enum class AppDebugMode {
    NONE      = 0,
    WIREFRAME = 1,
};

struct Application final {
    ~Application() { XLOG("Disposing application"); }
    engine::FirstPersonLocomotion cameraMovement{};
    engine::FirstPersonLocomotion debugCameraMovement{};
    // glm::vec3 cameraEulerRotation{0.0f, 0.0f, 0.0f};
    glm::vec4 keyboardWasdPressed{0.0f};
    float keyboardShiftPressed{0.0f};
    float keyboardAltPressed{0.0f};
    bool controlDebugCamera{false};
    bool controlDebugCameraSwitched{false};
    engine::gl::GlContext gl{};
    engine::gl::GpuMesh boxMesh{};
    engine::gl::GpuMesh sphereMesh{};
    engine::gl::GpuMesh sphereMesh2{};
    engine::gl::GpuMesh planeMesh{};
    engine::gl::GpuProgramHandle program{};
    engine::gl::Texture texture{};
    engine::gl::GpuBuffer uboSamplerTiling{};
    UboDataSamplerTiling uboDataSamplerTiling{};
    engine::gl::Framebuffer outputFramebuffer{};
    engine::gl::Texture outputColor{};
    engine::gl::Texture outputDepth{};
    engine::gl::Texture backbufferColor{};
    engine::gl::Texture backbufferDepth{};
    engine::gl::Renderbuffer renderbuffer{};
    engine::gl::CommonRenderers commonRenderers{};
    engine::gl::FlatRenderer flatRenderer{};
    engine::gl::SamplersCache::CacheKey samplerNearestWrap{};
    engine::LineRendererInput debugLines{};
    engine::PointRendererInput debugPoints{};
    engine::ImageLoader imageLoader{};
    AppDebugMode debugMode{AppDebugMode::NONE};

    engine::platform::FileChangeNotifier fileNotifier{};
    bool isInitialized = false;
};

struct ApplicationState {
    engine::EngineHandle engine;
    std::shared_ptr<engine::EnginePersistentData> engineData;
    std::unique_ptr<Application> app;
};

} // namespace app

CR_EXPORT auto cr_main(cr_plugin* ctx, cr_op operation) -> int;