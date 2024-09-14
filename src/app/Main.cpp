#include "engine/Assets.hpp"
#include "engine/BoxMesh.hpp"
#include "engine/EngineLoop.hpp"
#include "engine/FirstPersonLocomotion.hpp"
#include "engine/IcosphereMesh.hpp"
#include "engine/PlaneMesh.hpp"
#include "engine/Unprojection.hpp"
#include "engine/UvSphereMesh.hpp"
#include "engine/gl/Buffer.hpp"
#include "engine/gl/CommonRenderers.hpp"
#include "engine/gl/FlatRenderer.hpp"
#include "engine/gl/Framebuffer.hpp"
#include "engine/gl/Guard.hpp"
#include "engine/gl/Init.hpp"
#include "engine/gl/ProceduralMeshes.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Renderbuffer.hpp"
#include "engine/gl/Sampler.hpp"
#include "engine/gl/SamplersCache.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/gl/Texture.hpp"
#include "engine/gl/TextureUnits.hpp"
#include "engine/gl/Uniform.hpp"
#include "engine/gl/Vao.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define ENABLE_RENDERDOC 0

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <cr.h>
#pragma clang diagnostic pop

using ExternalUpdateCallback = bool (*)(engine::WindowCtx const&);
ExternalUpdateCallback g_externalUpdate;
using ExternalDestroyCallback = void (*)();
ExternalDestroyCallback g_externalDestroyCallback;

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
    ~Application() {
        XLOG("Disposing application", 0);
        engine::gl::DisposeOpenGl();
    }
    engine::FirstPersonLocomotion cameraMovement{};
    engine::FirstPersonLocomotion debugCameraMovement{};
    // glm::vec3 cameraEulerRotation{0.0f, 0.0f, 0.0f};
    glm::vec4 keyboardWasdPressed{0.0f};
    float keyboardShiftPressed{0.0f};
    float keyboardAltPressed{0.0f};
    bool controlDebugCamera{false};
    bool controlDebugCameraSwitched{false};
    engine::gl::GpuMesh boxMesh{};
    engine::gl::GpuMesh sphereMesh{};
    engine::gl::GpuMesh sphereMesh2{};
    engine::gl::GpuMesh planeMesh{};
    engine::gl::GpuProgram program{};
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

    bool isInitialized = false;
};

constexpr GLint ATTRIB_POSITION_LOCATION   = 0;
constexpr GLint ATTRIB_UV_LOCATION         = 1;
constexpr GLint ATTRIB_NORMAL_LOCATION     = 2;
constexpr GLint UNIFORM_TEXTURE_LOCATION   = 0;
constexpr GLint UNIFORM_TEXTURE_BINDING    = 0;
constexpr GLint UNIFORM_MVP_LOCATION       = 10;
constexpr GLint UBO_SAMPLER_TILING_BINDING = 4;

static void ConfigureApplication(engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx, std::unique_ptr<Application>& app) {
    using namespace engine;
    glm::ivec2 maxScreenSize = windowCtx.WindowSize() * 4;
    gl::InitializeOpenGl();
    app->commonRenderers.Initialize();
    app->samplerNearestWrap = app->commonRenderers.CacheSampler(
        "repeat/nearest",
        gl::GpuSampler::Allocate("Sampler/NearestRepeat")
            .WithLinearMagnify(false)
            .WithLinearMinify(false)
            .WithWrap(GL_REPEAT));

    gl::shader::Define const defines[] = {
        {.name = "ATTRIB_POSITION_LOCATION", .value = ATTRIB_POSITION_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "ATTRIB_UV_LOCATION", .value = ATTRIB_UV_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "UNIFORM_MVP_LOCATION", .value = UNIFORM_MVP_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "UNIFORM_TEXTURE_LOCATION", .value = UNIFORM_TEXTURE_LOCATION, .type = gl::shader::Define::INT32},
        {.name = "UNIFORM_TEXTURE_BINDING", .value = UNIFORM_TEXTURE_BINDING, .type = gl::shader::Define::INT32},
        {.name = "UBO_SAMPLER_TILING_BINDING", .value = UBO_SAMPLER_TILING_BINDING, .type = gl::shader::Define::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/app/shaders/triangle.vert", "data/app/shaders/texture.frag", CpuView{defines, std::size(defines)},
        "Test program");
    assert(maybeProgram);
    app->program = std::move(*maybeProgram);

    app->boxMesh = gl::AllocateBoxMesh(
        BoxMesh::Generate(VEC_ONES, true),
        gl::GpuMesh::AttributesLayout{
            .positionLocation = ATTRIB_POSITION_LOCATION,
            .uvLocation       = ATTRIB_UV_LOCATION,
            .normalLocation   = ATTRIB_NORMAL_LOCATION,
        });

    auto sphere     = UvSphereMesh::Generate({
            .numMeridians       = 7,
            .numParallels       = 4,
            .clockwiseTriangles = false,
    });
    app->sphereMesh = gl::AllocateUvSphereMesh(
        sphere,
        gl::GpuMesh::AttributesLayout{
            .positionLocation = ATTRIB_POSITION_LOCATION,
            .uvLocation       = ATTRIB_UV_LOCATION,
            .normalLocation   = ATTRIB_NORMAL_LOCATION,
        });
    app->sphereMesh2 = gl::AllocateIcosphereMesh(
        IcosphereMesh::Generate({
            .numSubdivisions    = 1,
            .duplicateSeam      = false,
            .clockwiseTriangles = true,
        }),
        gl::GpuMesh::AttributesLayout{
            .positionLocation = ATTRIB_POSITION_LOCATION,
            .uvLocation       = ATTRIB_UV_LOCATION,
            .normalLocation   = ATTRIB_NORMAL_LOCATION,
        });

    glm::ivec2 planeSize{8, 15};
    auto planeMesh = PlaneMesh::Generate(planeSize, glm::vec2{0.5f, 0.5f});
    app->planeMesh = gl::AllocatePlaneMesh(
        planeMesh,
        gl::GpuMesh::AttributesLayout{
            .positionLocation = ATTRIB_POSITION_LOCATION,
            .uvLocation       = ATTRIB_UV_LOCATION,
            .normalLocation   = ATTRIB_NORMAL_LOCATION,
        });

    auto const& debugMesh = sphere;
    // app.debugPoints.SetColor(ColorCode::RED);
    // for (int32_t i = 0; i < debugMesh.vertexPositions.size(); ++i) {
    //     app.debugPoints.PushPoint(debugMesh.vertexPositions[i], 0.03f);
    // }
    // for (int32_t i = 0; i < 12 /* debugMesh.indices.size() / 3 */; ++i) {
    //     auto vi0 = debugMesh.indices[3*i];
    //     auto vi1 = debugMesh.indices[3*i+1];
    //     auto vi2 = debugMesh.indices[3*i+2];
    //     app.debugPoints.SetColor(static_cast<ColorCode>((i) % static_cast<int32_t>(ColorCode::NUM_COLORS)));
    //     app.debugPoints.PushPoint(debugMesh.vertexPositions[vi0], 0.03f);
    //     app.debugPoints.PushPoint(debugMesh.vertexPositions[vi1], 0.03f);
    //     app.debugPoints.PushPoint(debugMesh.vertexPositions[vi2], 0.03f);
    // }

    auto maybeTexture = gl::LoadTexture(engine::gl::LoadTextureArgs{
        .loader      = app->imageLoader,
        .filepath    = "data/engine/textures/utils/uv_checker_8x8_bright.png",
        .format      = GL_SRGB8,
        .numChannels = 3,
    });
    assert(maybeTexture);

    app->texture          = std::move(*maybeTexture);
    app->uboSamplerTiling = gl::GpuBuffer::Allocate(
        GL_UNIFORM_BUFFER, GL_STREAM_DRAW, CpuMemory<GLvoid const>{nullptr, sizeof(UboDataSamplerTiling)},
        "SamplerTiling UBO");
    app->uboDataSamplerTiling.albedoIdx = 42;
    gl::SamplerTiling albedoTiling{glm::vec2{0.25f}, glm::vec2{0.0f}};
    app->uboDataSamplerTiling.uvScaleOffsets[app->uboDataSamplerTiling.albedoIdx] = albedoTiling.Packed();
    app->uboSamplerTiling.Fill(CpuMemory<GLvoid const>{&app->uboDataSamplerTiling, sizeof(app->uboDataSamplerTiling)});

    // GL_RGB10_A2, GL_R11F_G11F_B10F, GL_RGBA16F, GL_RGBA8
    app->outputColor = gl::Texture::Allocate2D(
        GL_TEXTURE_2D, glm::ivec3(maxScreenSize.x, maxScreenSize.y, 0), GL_RGBA8, "Output/Color");
    app->outputDepth = gl::Texture::Allocate2D(
        GL_TEXTURE_2D, glm::ivec3(maxScreenSize.x, maxScreenSize.y, 0), GL_DEPTH24_STENCIL8, "Output/Depth");
    // app->renderbuffer      = gl::Renderbuffer::Allocate2D(maxScreenSize, GL_DEPTH24_STENCIL8, 0, "Test
    // renderbuffer");
    app->outputFramebuffer = gl::Framebuffer::Allocate("Main Pass FBO");
    (void)gl::FramebufferEditCtx{app->outputFramebuffer}
        .AttachTexture(GL_COLOR_ATTACHMENT0, app->outputColor)
        .AttachTexture(GL_DEPTH_STENCIL_ATTACHMENT, app->outputDepth)
        // .AttachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, app->renderbuffer)
        .CommitDrawbuffers();

    app->flatRenderer = gl::FlatRenderer::Allocate();

    app->cameraMovement.SetPosition({0.0f, 10.0f, 2.0f});
    app->cameraMovement.SetOrientation(VEC_FORWARD, VEC_UP);
}

static void Render(engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx, void* appData) {
    using namespace engine;
    auto appPtr = static_cast<std::unique_ptr<Application>*>(appData);
    if (!appPtr) [[unlikely]] {
        return;
    }
    auto& app = *appPtr;
    if (!app->isInitialized) [[unlikely]] {
        ConfigureApplication(ctx, windowCtx, app);
        app->isInitialized = true;
    }

    glm::ivec2 renderSize = windowCtx.WindowSize();
    // glm::ivec2 renderSize = glm::vec2{windowCtx.WindowSize()} * (glm::sin(ctx.timeSec) * 0.7f + 1.0f);
    glm::ivec2 screenSize = windowCtx.WindowSize();
    float aspectRatio     = static_cast<float>(screenSize.x) / static_cast<float>(screenSize.y);

    if (app->controlDebugCameraSwitched) {
        if (app->controlDebugCamera) { app->debugCameraMovement.Clone(app->cameraMovement); }
        app->controlDebugCameraSwitched = false;
    }
    auto& cameraMovement = app->controlDebugCamera ? app->debugCameraMovement : app->cameraMovement;

    // TODO: delta time
    float CAMERA_MOVE_SENSITIVITY = 0.1f;
    float moveSensitivity         = (1.0f - 0.75f * app->keyboardAltPressed) * CAMERA_MOVE_SENSITIVITY;
    glm::vec3 cameraDeltaPosition{};
    if (app->keyboardShiftPressed) {
        cameraDeltaPosition.y = (app->keyboardWasdPressed.x - app->keyboardWasdPressed.z) * 0.5f;
    } else {
        glm::vec2 cameraMoveDir(
            (app->keyboardWasdPressed.w - app->keyboardWasdPressed.y),
            (app->keyboardWasdPressed.x - app->keyboardWasdPressed.z));
        float invDirLength = glm::inversesqrt(glm::dot(cameraMoveDir, cameraMoveDir));
        if (invDirLength < 1.0f) { cameraMoveDir *= invDirLength; }
        cameraDeltaPosition.x = cameraMoveDir.x;
        cameraDeltaPosition.z = cameraMoveDir.y;
    }
    cameraMovement.MoveLocally(cameraDeltaPosition * moveSensitivity);

    auto mouse = windowCtx.MousePressedState();
    // XLOG("@@ Mouse {} {} {}", mouse.x, mouse.y, mouse.z);
    constexpr float CAMERA_ROTATION_SENSITIVITY = 0.05f;
    if (mouse.x > 0.0f) {
        auto mouseDelta = windowCtx.MouseDelta();
        glm::quat deltaRotation; // euler pitch+yaw to quat
        glm::vec2 yawPitch{mouseDelta.x, -mouseDelta.y * aspectRatio};
        glm::vec2 quatAngle = glm::radians(yawPitch * 0.5f); // quat angle is half the angle of rotation
        glm::vec2 cos       = glm::cos(quatAngle);
        glm::vec2 sin       = glm::sin(quatAngle);
        deltaRotation.w     = cos.y * cos.x;
        deltaRotation.x     = sin.y * sin.x;
        deltaRotation.y     = sin.y * cos.x;
        deltaRotation.z     = cos.y * sin.x;
        deltaRotation       = glm::normalize(deltaRotation);

        cameraMovement.RotateLocally(
            glm::radians(glm::vec2{mouseDelta.x, -mouseDelta.y}) * CAMERA_ROTATION_SENSITIVITY);
        // cameraMovement.RotateLocally(deltaRotation * CAMERA_ROTATION_SENSITIVITY);
    }

    cameraMovement.CommitChanges();
    glm::mat4 view = cameraMovement.ComputeViewMatrix();
    glm::mat4 proj = glm::perspective(glm::radians(30.0f), aspectRatio, 1.0f, 200.0f);

    glm::mat4 camera = proj * view;

    if (app->debugMode == AppDebugMode::WIREFRAME) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }

    float rotationSpeed = ctx.timeSec * 0.5f;

    auto fbGuard = gl::FramebufferDrawCtx{app->outputFramebuffer};
    {
        // textured box
        fbGuard.ClearColor(0, 0.1f, 0.2f, 0.3f, 0.0f);
        fbGuard.ClearDepthStencil(1.0f, 0);
        GLCALL(glViewport(0, 0, renderSize.x, renderSize.y));

        glm::mat4 model = glm::mat4(1.0f);
        // model           = glm::rotate(model, rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
        // model = glm::scale(model, glm::vec3(1.0f, 1.0f, 0.001f));
        model = glm::translate(model, glm::vec3(3.0f, 3.0f, 0.0f));

        auto debugGroupGuard = gl::DebugGroupCtx("Main pass");

        glm::mat4 mvp = camera * model;

        GLCALL(glEnable(GL_CULL_FACE));
        GLCALL(glEnable(GL_DEPTH_TEST));
        GLCALL(glDepthMask(GL_TRUE));
        GLCALL(glFrontFace(GL_CCW));
        GLCALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

        app->commonRenderers.RenderAxes(mvp, 0.4f, ColorCode::CYAN);

        constexpr GLint TEXTURE_SLOT = 0;
        auto programGuard            = gl::UniformCtx(app->program);
        programGuard.SetUniformTexture(UNIFORM_TEXTURE_LOCATION, TEXTURE_SLOT);
        programGuard.SetUniformMatrix4x4(UNIFORM_MVP_LOCATION, glm::value_ptr(mvp));
        GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_SAMPLER_TILING_BINDING, app->uboSamplerTiling.Id()));
        gl::GlTextureUnits::Bind2D(TEXTURE_SLOT, app->texture.Id());
        // gl::GlTextureUnits::Bind2D(TEXTURE_SLOT, app->commonRenderers.TextureStubColor().Id());
        gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, app->commonRenderers.FindSampler(app->samplerNearestWrap).Id());
        gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, app->commonRenderers.SamplerLinearRepeat().Id());

        gl::RenderVao(app->planeMesh.Vao(), GL_TRIANGLE_STRIP);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
        model = glm::translate(model, VEC_ONES);
        programGuard.SetUniformMatrix4x4(UNIFORM_MVP_LOCATION, glm::value_ptr(camera * model));

        // if (windowCtx.IsMouseInsideWindow()) {
        //     gl::RenderVao(app->sphereMesh.Vao());
        // } else {
        //     gl::RenderVao(app->sphereMesh2.Vao());
        // }

        gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, 0);
    }

    {
        // lighted box
        GLCALL(glViewport(0, 0, renderSize.x, renderSize.y));

        float lightRadius    = 2.0f;
        glm::mat4 lightModel = glm::mat4{1.0f};
        lightModel           = glm::rotate(lightModel, rotationSpeed * 5.5f, VEC_UP);
        lightModel =
            glm::translate(lightModel, glm::vec3(lightRadius, lightRadius, lightRadius /* * glm::sin(ctx.timeSec) */ + 1.0f));
        glm::vec3 lightPosition{gl::TransformOrigin(lightModel)};

        glm::mat4 model = glm::mat4(1.0f);
        // model           = glm::rotate(model, glm::pi<float>() * 0.1f, glm::vec3(0.0f, 0.0f, 1.0f));
        // float modelScale = 1.0f;
        // model            = glm::scale(model, glm::vec3(1.0f, 1.0f, 200.0f));
        // model            = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 mvp = camera * model;

        // app->commonRenderers.RenderAxes(mvp, 1.5f, ColorCode::WHITE);
        app->commonRenderers.RenderAxes(camera, 0.4f, ColorCode::BROWN);
        app->commonRenderers.RenderAxes(camera * lightModel, 0.2f, ColorCode::YELLOW);

        gl::GpuMesh const& mesh = app->sphereMesh;
        GLCALL(glEnable(GL_CULL_FACE));
        GLCALL(glEnable(GL_DEPTH_TEST));
        GLCALL(glFrontFace(mesh.FrontFace()));
        GLCALL(glDepthMask(GL_TRUE));
        GLCALL(glDepthFunc(GL_LEQUAL));

        glm::vec3 lightColor{0.2f, 0.2f, 0.2f};
        app->flatRenderer.Render(gl::FlatRenderArgs{
            .lightWorldPosition        = lightPosition,
            .lightColor                = lightColor,
            .eyeWorldPosition          = cameraMovement.Position(),
            .materialColor             = glm::vec3{0.3, 1.0, 0.1},
            .materialSpecularIntensity = 1.0f,
            .primitive                 = GL_TRIANGLES,
            .vaoWithNormal             = mesh.Vao(),
            .mvp                       = mvp,
            .modelToWorld              = model,
        });

        model = glm::scale(glm::mat4{1.0f}, glm::vec3{15.0f});
        mvp   = camera * model;
        app->flatRenderer.Render(gl::FlatRenderArgs{
            .lightWorldPosition        = lightPosition,
            .lightColor                = lightColor,
            .eyeWorldPosition          = cameraMovement.Position(),
            // .materialColor             = glm::vec3{1.0f, 1.0f, 1.0f},
            .materialSpecularIntensity = 1.0f,
            .primitive                 = GL_TRIANGLES,
            .vaoWithNormal             = app->boxMesh.Vao(),
            .mvp                       = mvp,
            .modelToWorld              = model,
        });
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    {
        auto debugGroupGuard = gl::DebugGroupCtx("Debug pass");

        glm::mat4 model{1.0};
        model = glm::rotate(model, rotationSpeed * 0.5f, VEC_UP);
        model = glm::translate(model, VEC_RIGHT * 1.6f);

        glm::mat4 mvp = camera * model;
        app->commonRenderers.RenderBox(camera * model, glm::vec4(0.2f, 1.0f, 0.2f, 1.0f));

        if (app->controlDebugCamera) {
            Frustum frustum = ProjectionToFrustum(proj);
            auto frustumMvp = camera * app->cameraMovement.ComputeModelMatrix();
            app->commonRenderers.RenderFrustum(frustumMvp, frustum, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f), 0.02f);
            app->commonRenderers.RenderAxes(frustumMvp, 0.5f, ColorCode::BLACK);
        }

        {
            // glm::mat4 mvp = camera * model;
            glm::vec2 billboardSize        = glm::vec2{1.0f, 1.0f};
            glm::vec3 billboardPivotOffset = glm::vec3{0.0f, 0.0f, 0.0f};
            gl::ScreenShaderArgs screen{
                .pixelsPerUnitX = 0.001f * static_cast<float>(screenSize.x),
                .pixelsPerUnitY = 0.001f * static_cast<float>(screenSize.y),
                .pixelsHeight   = static_cast<float>(screenSize.y),
                .aspectRatio    = aspectRatio};
            app->commonRenderers.RenderBillboard(gl::BillboardRenderArgs{
                app->commonRenderers.VaoDatalessQuad(),
                GL_TRIANGLE_STRIP,
                screen,
                mvp,
                billboardSize,
                billboardPivotOffset,
            });
        }

        app->commonRenderers.RenderAxes(mvp, 0.5f, ColorCode::WHITE);

        gl::RenderVao(app->commonRenderers.VaoDatalessQuad(), GL_POINTS);
    }

    fbGuard.GuardAnother(0U);
    {
        // present
        glViewport(0, 0, screenSize.x, screenSize.y);
        fbGuard.ClearDepthStencil(1.0f, 0);
        // GLenum invalidateAttachments[1] = {GL_COLOR_ATTACHMENT0};
        // .Invalidate(1, invalidateAttachments);
        glm::vec2 fractionOfMaxResolution = glm::vec2{renderSize} / glm::vec2{app->outputColor.Size()};
        app->commonRenderers.Blit2D(app->outputColor.Id(), fractionOfMaxResolution);
    }

    {
        GLCALL(glEnable(GL_CULL_FACE));
        GLCALL(glEnable(GL_DEPTH_TEST));
        GLCALL(glDepthMask(GL_TRUE));
        GLCALL(glDepthFunc(GL_LEQUAL));

        auto debugGroupGuard = gl::DebugGroupCtx("Debug lines/points pass");
        if (app->debugLines.IsDataDirty()) {
            app->commonRenderers.FlushLinesToGpu(app->debugLines.Data());
            app->debugLines.Clear();
        }
        app->commonRenderers.RenderLines(camera);

        if (app->debugPoints.IsDataDirty()) {
            app->commonRenderers.FlushPointsToGpu(app->debugPoints.Data());
            app->debugPoints.Clear();
        }
        app->commonRenderers.RenderPoints(camera);
    }

    app->commonRenderers.OnFrameEnd();
    gl::GlTextureUnits::RestoreState();
}

static auto ConfigureWindow(engine::EngineHandle engine) {
    auto& windowCtx    = engine::GetWindowContext(engine);
    GLFWwindow* window = windowCtx.Window();
    using KeyModFlags  = engine::WindowCtx::KeyModFlags;
    auto& app = *static_cast<std::unique_ptr<Application>*>(engine::GetApplicationData(engine));
    (void)windowCtx.SetKeyboardCallback(GLFW_KEY_W, [&](bool pressed, bool released, KeyModFlags mods) {
        app->keyboardWasdPressed.x += static_cast<float>(pressed) - static_cast<float>(released);
    });
    (void)windowCtx.SetKeyboardCallback(GLFW_KEY_A, [&](bool pressed, bool released, KeyModFlags) {
        app->keyboardWasdPressed.y += static_cast<float>(pressed) - static_cast<float>(released);
    });
    (void)windowCtx.SetKeyboardCallback(GLFW_KEY_S, [&](bool pressed, bool released, KeyModFlags) {
        app->keyboardWasdPressed.z += static_cast<float>(pressed) - static_cast<float>(released);
    });
    (void)windowCtx.SetKeyboardCallback(GLFW_KEY_D, [&](bool pressed, bool released, KeyModFlags) {
        app->keyboardWasdPressed.w += static_cast<float>(pressed) - static_cast<float>(released);
    });

    (void)windowCtx.SetKeyboardCallback(GLFW_KEY_Q, [&](bool pressed, bool released, KeyModFlags) {
        static bool controlDebugCamera = true;
        if (pressed) {
            app->controlDebugCameraSwitched = true;
            app->controlDebugCamera         = controlDebugCamera;
            controlDebugCamera              = !controlDebugCamera;
        }
    });

    (void)windowCtx.SetKeyboardCallback(GLFW_KEY_LEFT_ALT, [&](bool pressed, bool released, KeyModFlags) {
        app->keyboardAltPressed += static_cast<float>(pressed) - static_cast<float>(released);
    });

    (void)windowCtx.SetKeyboardCallback(GLFW_KEY_LEFT_SHIFT, [&](bool pressed, bool released, KeyModFlags) {
        app->keyboardShiftPressed += static_cast<float>(pressed) - static_cast<float>(released);
    });

    (void)windowCtx.SetKeyboardCallback(GLFW_KEY_ESCAPE, [&](bool pressed, bool released, KeyModFlags) {
        engine::QueueForNextFrame(
            engine, engine::UserActionType::WINDOW, [=](void*) { glfwSetWindowShouldClose(window, true); });
    });

    (void)windowCtx.SetKeyboardCallback(GLFW_KEY_F, [&](bool pressed, bool released, KeyModFlags) {
        engine::QueueForNextFrame(engine, engine::UserActionType::WINDOW, [=](void*) {
            static bool setToFullscreen = true;
            if (!pressed) { return; }

            GLFWmonitor* monitor    = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            if (setToFullscreen) {
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            } else {
                // TODO: avoid hardcoding resolution
                glfwSetWindowMonitor(window, nullptr, 0, 0, 800, 600, mode->refreshRate);
            }
            XLOG("Fullscreen mode: {}", setToFullscreen);
            setToFullscreen = !setToFullscreen;
        });
    });

    (void)windowCtx.SetKeyboardCallback(GLFW_KEY_P, [&](bool pressed, bool released, KeyModFlags) {
        static bool setToWireframe = true;
        if (!pressed) { return; }

        if (setToWireframe) {
            app->debugMode = AppDebugMode::WIREFRAME;
        } else {
            // TODO: this resets all debug modes
            app->debugMode = AppDebugMode::NONE;
        }
        XLOG("Wireframe mode: {}", setToWireframe);
        setToWireframe = !setToWireframe;
    });

    (void)windowCtx.SetMouseButtonCallback(GLFW_MOUSE_BUTTON_LEFT, [&](bool pressed, bool released, KeyModFlags) {
        if (released) {
            auto mousePosition = windowCtx.MousePosition();
            XLOG("LMB {} pos: {},{}", windowCtx.IsMouseInsideWindow(), mousePosition.x, mousePosition.y);
        }
    });
}

struct ApplicationState {
    engine::EngineHandle engine;
    std::shared_ptr<engine::EnginePersistentData> engineData;
    std::unique_ptr<Application> app;
} g_app;

auto ColdStartApplication[[nodiscard]](ApplicationState& destination) -> engine::EngineError {
    XLOG("! Compiled in DEBUG mode", 0);

    assert(!destination.app);
    destination.engine = engine::CreateEngine();
    if (auto result = engine::ColdStartEngine(destination.engine); result != engine::EngineError::SUCCESS) {
        return result;
    }
    destination.app = std::make_unique<Application>();
    engine::SetApplicationData(destination.engine, &destination.app);

    ConfigureWindow(destination.engine);

    engine::QueueForNextFrame(
        destination.engine, engine::UserActionType::RENDER, [](void* applicationData) { GLCALL(glEnable(GL_FRAMEBUFFER_SRGB)); });

    auto _ = engine::SetRenderCallback(destination.engine, Render);
    return engine::EngineError::SUCCESS;
}

auto DestroyApplication(ApplicationState& destination) -> bool {
    // if (g_externalDestroyCallback) {
    //     g_externalDestroyCallback();
    // }
    destination.app.reset();
    destination.engineData.reset();
    (void)engine::DestroyEngine(destination.engine);
    return true;
}

// Hot reloading "guest" part
CR_EXPORT auto cr_main(cr_plugin *ctx, cr_op operation) -> int {
    assert(ctx != nullptr);
    static ApplicationState* state{nullptr};
    switch (operation) {
        case CR_LOAD:
            XLOGW("HotReload::load v{} e{}", ctx->version, static_cast<int32_t>(ctx->failure));
            if (state == nullptr) {
                state = reinterpret_cast<ApplicationState*>(ctx->userdata);
            }
            assert(state->engine == engine::ENGINE_HANDLE_NULL);
            state->engine = engine::CreateEngine();
            if (state->engineData) {
                return static_cast<int>(engine::HotStartEngine(state->engine, state->engineData));
            }
            return static_cast<int>(ColdStartApplication(*state));
        case CR_STEP:
            return static_cast<int>(engine::TickEngine(state->engine));
        case CR_UNLOAD:
            // preparing to a new reload
            XLOGW("HotReload::unload v{} e{}", ctx->version, static_cast<int32_t>(ctx->failure));
            state->engineData = engine::DestroyEngine(state->engine);
            state->engine = engine::ENGINE_HANDLE_NULL;
            return 0;
        case CR_CLOSE:
            // the plugin will close and not reload anymore
            XLOGW("HotReload::destroy v{}", ctx->version);
            return static_cast<int>(DestroyApplication(*state));
    }
}

auto main() -> int {
    // emulate context of hot reloading library CR
    cr_plugin crCtx {
        .p = nullptr,
        .userdata = &g_app,
        .version = 0,
        .failure = CR_NONE,
        .next_version = 1,
        .last_working_version = 0,
    };

    if (auto result = cr_main(&crCtx, CR_LOAD); result != 0) {
        return result;
    }
    // if (auto result = ColdStartApplication(g_app); result != engine::EngineError::SUCCESS) {
    //     return static_cast<int>(result);
    // }
    while(true){
        // if (g_externalUpdate) [[likely]] {
        //     g_externalUpdate( windowCtx);
        // }
        if (auto result = cr_main(&crCtx, CR_STEP); result != 0) {
            return result;
        }
        // auto tickResult = engine::TickEngine(g_app.engine);
        // if (tickResult != engine::EngineError::SUCCESS) {
        //     break;
        // }
    }
    if (auto result = cr_main(&crCtx, CR_UNLOAD); result != 0) {
        return result;
    }
    // if (DestroyApplication(g_app) == false) {
    //     return 2;
    // }

    return 0;
}