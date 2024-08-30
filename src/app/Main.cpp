#include "engine/Assets.hpp"
#include "engine/BoxMesh.hpp"
#include "engine/EngineLoop.hpp"
#include "engine/IcosphereMesh.hpp"
#include "engine/PlaneMesh.hpp"
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

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define ENABLE_RENDERDOC 0

struct UboDataSamplerTiling {
    GLint albedoIdx   = 0;
    GLint normalIdx   = 0;
    GLint specularIdx = 0;
    GLint __pad0      = 0;
    glm::vec4 uvScaleOffsets[256U];
};

struct Application final {
    ~Application() {
        XLOG("Disposing application", 0);
        engine::gl::DisposeOpenGl();
    }

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
    engine::gl::Renderbuffer renderbuffer{};
    engine::gl::CommonRenderers commonRenderers{};
    engine::gl::FlatRenderer flatRenderer{};
    engine::gl::SamplersCache::CacheKey samplerNearestWrap{};
    engine::LineRendererInput debugLines{};
    engine::PointRendererInput debugPoints{};
    engine::ImageLoader imageLoader{};

    bool isInitialized = false;
};

constexpr uint8_t textureData[] = {
    80,  40,  40,  // 01
    80,  120, 80,  // 03
    120, 120, 160, // 11
    200, 160, 160, // 13
};

constexpr GLint ATTRIB_POSITION_LOCATION           = 0;
constexpr GLint ATTRIB_UV_LOCATION                 = 1;
constexpr GLint ATTRIB_NORMAL_LOCATION             = 2;
constexpr GLint UNIFORM_TEXTURE_LOCATION           = 0;
constexpr GLint UNIFORM_TEXTURE_BINDING           = 0;
constexpr GLint UNIFORM_MVP_LOCATION               = 10;
constexpr GLint UBO_SAMPLER_TILING_BINDING = 4;
constexpr glm::ivec2 INTERMEDITE_RENDER_RESOLUTION = glm::ivec2(1600, 900);

static void InitializeApplication(engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx, Application* app) {
    using namespace engine;
    glm::ivec2 screenSize = INTERMEDITE_RENDER_RESOLUTION;
    gl::InitializeOpenGl();
    app->commonRenderers.Initialize();
    app->samplerNearestWrap = app->commonRenderers.CacheSampler(
        "repeat/nearest",
        gl::GpuSampler::Allocate("Sampler/NearestRepeat")
            .WithLinearMagnify(false)
            .WithLinearMinify(false)
            .WithWrap(GL_REPEAT));

    gl::ShaderDefine const defines[] = {
        {.name = "ATTRIB_POSITION_LOCATION", .value = ATTRIB_POSITION_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_UV_LOCATION", .value = ATTRIB_UV_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_MVP_LOCATION", .value = UNIFORM_MVP_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_TEXTURE_LOCATION", .value = UNIFORM_TEXTURE_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_TEXTURE_BINDING", .value = UNIFORM_TEXTURE_BINDING, .type = gl::ShaderDefine::INT32},
        {.name = "UBO_SAMPLER_TILING_BINDING", .value = UBO_SAMPLER_TILING_BINDING, .type = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/app/shaders/triangle.vert", "data/app/shaders/texture.frag", CpuView{defines, std::size(defines)},
        "Test program");
    assert(maybeProgram);
    app->program = std::move(*maybeProgram);

    auto boxCpuMesh = BoxMesh::Generate(glm::vec3{2.5f, 2.5f, 1.0f}, false);
    app->boxMesh    = gl::AllocateBoxMesh(
        boxCpuMesh,
        gl::GpuMesh::AttributesLayout{
               .positionLocation = ATTRIB_POSITION_LOCATION,
               .uvLocation       = ATTRIB_UV_LOCATION,
               .normalLocation   = ATTRIB_NORMAL_LOCATION,
        });

    auto uvSphere = UvSphereMesh::Generate({
        .numMeridians       = 20,
        .numParallels       = 20,
        .clockwiseTriangles = false,
    });

    app->sphereMesh = gl::AllocateUvSphereMesh(
        uvSphere,
        gl::GpuMesh::AttributesLayout{
            .positionLocation = ATTRIB_POSITION_LOCATION,
            .uvLocation       = ATTRIB_UV_LOCATION,
            .normalLocation   = ATTRIB_NORMAL_LOCATION,
        });
    app->sphereMesh2 = gl::AllocateIcosphereMesh(
        IcosphereMesh::Generate({
            .numSubdivisions    = 1,
            .duplicateSeam      = false,
            .clockwiseTriangles = false,
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

    // for (int i = 0; i < planeMesh.vertexData.size(); ++i) {
    //     auto ii = i % planeMesh.vertexData.size();
    //     app->debugLines.SetColor(static_cast<ColorCode>(i % static_cast<int>(ColorCode::NUM_COLORS)));
    //     app->debugLines.PushRay(planeMesh.vertexPositions[ii], planeMesh.vertexData[ii].normal * 0.2f);
    // }
    // app->debugLines.SetTransform(glm::translate(glm::mat4{1.0f}, glm::vec3{-1.0f, 0.0f, 0.0f}));
    // for (int i = 0; i < planeMesh.vertexData.size(); ++i) {
    //     // app->debugLines.SetColor(static_cast<ColorCode>(i % static_cast<int>(ColorCode::NUM_COLORS)));
    //     app->debugLines.PushRay(
    //         planeMesh.vertexPositions[i],
    //         glm::cross(planeMesh.vertexData[i].normal, glm::vec3{0.0f, 0.0f, -1.0f}) * -0.2f);
    // }
    int off = 1;
    for (int i = off; i < off + 8 /* planeMesh.indices.size() */; ++i) {
        // if (i % planeSize.x == 0) {
        //     app->debugPoints.SetColor(static_cast<ColorCode>(
        //         (i / planeSize.x)  % static_cast<int>(ColorCode::NUM_COLORS)));
        // }
        auto vi = planeMesh.indices[i];
        app->debugPoints.SetColor(static_cast<ColorCode>((i - off) % 4));
        app->debugPoints.PushPoint(planeMesh.vertexPositions[vi], 0.03f);
    }

    auto maybeTexture = gl::LoadTexture(engine::gl::LoadTextureArgs{
        .loader = app->imageLoader,
        .filepath = "data/app/textures/shrek.jpeg",
        .format = GL_RGB8,
        .numChannels = 3,
    });
    assert(maybeTexture);

    app->texture = std::move(*maybeTexture);
    app->uboSamplerTiling = gl::GpuBuffer::Allocate(
        GL_UNIFORM_BUFFER, GL_STREAM_DRAW, nullptr, sizeof(UboDataSamplerTiling), "SamplerTiling UBO");
    app->uboDataSamplerTiling.albedoIdx = 42;
    gl::SamplerTiling albedoTiling{glm::vec2{1.0f}, glm::vec2{0.0f}};
    app->uboDataSamplerTiling.uvScaleOffsets[app->uboDataSamplerTiling.albedoIdx] = albedoTiling.Packed();
    app->uboSamplerTiling.Fill(&app->uboDataSamplerTiling, sizeof(app->uboDataSamplerTiling), 0);

    app->outputColor =
        gl::Texture::Allocate2D(GL_TEXTURE_2D, glm::ivec3(screenSize.x, screenSize.y, 0), GL_RGBA8, "Output color");
    app->outputDepth = gl::Texture::Allocate2D(
        GL_TEXTURE_2D, glm::ivec3(screenSize.x, screenSize.y, 0), GL_DEPTH24_STENCIL8, "Output depth");
    app->renderbuffer      = gl::Renderbuffer::Allocate2D(screenSize, GL_DEPTH24_STENCIL8, 0, "Test renderbuffer");
    app->outputFramebuffer = gl::Framebuffer::Allocate("Main Pass FBO");
    (void)gl::FramebufferCtx{app->outputFramebuffer, true}
        .LinkTexture(GL_COLOR_ATTACHMENT0, app->outputColor)
        // .LinkTexture(GL_DEPTH_STENCIL_ATTACHMENT, app->outputDepth);
        .LinkRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, app->renderbuffer);

    app->flatRenderer = gl::FlatRenderer::Allocate();
}
static void Render(engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx, void* appData) {
    using namespace engine;
    auto* app = static_cast<Application*>(appData);
    if (!app->isInitialized) {
        InitializeApplication(ctx, windowCtx, app);
        app->isInitialized = true;
    }

    glm::ivec2 renderSize = app->outputColor.Size();
    glm::ivec2 screenSize = windowCtx.WindowSize();
    float rotationSpeed   = ctx.timeSec * 0.5f;

    float cameraRadius    = 15.0f;
    glm::mat4 cameraModel = glm::translate(
        glm::mat4{1.0f}, glm::vec3(cameraRadius * glm::cos(ctx.timeSec), cameraRadius * glm::sin(ctx.timeSec), 3.0f));
    // cameraModel           = glm::rotate(cameraModel, rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec3 cameraPosition = gl::TransformOrigin(cameraModel);
    glm::vec3 cameraTarget   = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp       = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::mat4 view           = glm::lookAtRH(glm::vec3{cameraPosition}, cameraTarget, cameraUp);

    float aspectRatio = static_cast<float>(screenSize.x) / static_cast<float>(screenSize.y);
    glm::mat4 proj    = glm::perspective(glm::radians(30.0f), aspectRatio, 0.1f, 100.0f);

    glm::mat4 camera = proj * view;

    {
        // textured box

        auto fbGuard = gl::FramebufferCtx{app->outputFramebuffer, true};
        fbGuard      = fbGuard.ClearColor(0, 0.1f, 0.2f, 0.3f, 0.0f).ClearDepthStencil(1.0f, 0);
        GLCALL(glViewport(0, 0, renderSize.x, renderSize.y));

        glm::mat4 model = glm::mat4(1.0f);
        // model           = glm::rotate(model, rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 0.001f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

        auto debugGroupGuard = gl::DebugGroupCtx("Main pass");

        auto programGuard = gl::UniformCtx(app->program);

        glm::mat4 mvp                = camera * model;
        constexpr GLint TEXTURE_SLOT = 0;
        programGuard.SetUniformTexture(UNIFORM_TEXTURE_LOCATION, TEXTURE_SLOT);
        programGuard.SetUniformMatrix4x4(UNIFORM_MVP_LOCATION, glm::value_ptr(mvp));
        GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, UBO_SAMPLER_TILING_BINDING, app->uboSamplerTiling.Id()));
        gl::GlTextureUnits::Bind2D(TEXTURE_SLOT, app->texture.Id());
        // gl::GlTextureUnits::Bind2D(TEXTURE_SLOT, app->commonRenderers.TextureStubColor().Id());
        gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, app->commonRenderers.FindSampler(app->samplerNearestWrap).Id());
        gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, app->commonRenderers.SamplerLinearRepeat().Id());

        GLCALL(glEnable(GL_CULL_FACE));
        GLCALL(glEnable(GL_DEPTH_TEST));
        GLCALL(glDepthMask(GL_TRUE));
        GLCALL(glFrontFace(GL_CCW));
        GLCALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

        // gl::RenderVao(app->boxMesh.Vao());
        gl::RenderVao(app->planeMesh.Vao(), GL_TRIANGLE_STRIP);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
        model = glm::translate(model, glm::vec3(1.0f, 1.0f, 1.0f));
        programGuard.SetUniformMatrix4x4(UNIFORM_MVP_LOCATION, glm::value_ptr(camera * model));

        // if (windowCtx.MouseInsideWindow()) {
        //     gl::RenderVao(app->sphereMesh.Vao());
        // } else {
        //     gl::RenderVao(app->sphereMesh2.Vao());
        // }

        gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, 0);
    }

    {
        // lighted box
        GLCALL(glViewport(0, 0, renderSize.x, renderSize.y));
        auto fbGuard = gl::FramebufferCtx{app->outputFramebuffer, true};

        glm::mat4 lightModel = glm::mat4{1.0f};
        lightModel           = glm::rotate(lightModel, rotationSpeed * 5.5f, glm::vec3(0.0f, 0.0f, 1.0f));
        lightModel           = glm::translate(lightModel, glm::vec3(2.0f, 2.0f, 2.0f * glm::sin(ctx.timeSec) + 1.0f));
        glm::vec3 lightPosition = gl::TransformOrigin(lightModel);

        glm::mat4 model = glm::mat4(1.0f);
        model           = glm::rotate(model, glm::pi<float>() * 0.1f, glm::vec3(0.0f, 0.0f, 1.0f));
        // model = glm::rotate(model, rotationSpeed * 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        float modelScale = 1.0f;
        model            = glm::scale(model, glm::vec3(modelScale, modelScale, modelScale));
        model            = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat invModel = glm::inverse(model);
        glm::mat4 mvp     = camera * model;

        // app->commonRenderers.RenderAxes(mvp, 1.5f, ColorCode::WHITE);
        app->commonRenderers.RenderAxes(camera, 0.4f, ColorCode::BROWN);
        app->commonRenderers.RenderAxes(camera * lightModel, 0.5f, ColorCode::YELLOW);

        gl::GpuMesh const& mesh = app->sphereMesh;
        GLCALL(glEnable(GL_CULL_FACE));
        GLCALL(glEnable(GL_DEPTH_TEST));
        GLCALL(glFrontFace(mesh.FrontFace()));
        GLCALL(glDepthMask(GL_TRUE));
        GLCALL(glDepthFunc(GL_LEQUAL));

        app->flatRenderer.Render(gl::FlatRenderArgs{
            .lightWorldPosition = lightPosition,
            .primitive          = GL_TRIANGLES,
            .vaoWithNormal      = mesh.Vao(),
            .mvp                = mvp,
            .invModel           = invModel,
        });
    }

    {
        // present
        glViewport(0, 0, screenSize.x, screenSize.y);
        auto dstGuard = gl::FramebufferCtx{0U, true}.ClearDepthStencil(1.0f, 0);
        // GLenum invalidateAttachments[1] = {GL_COLOR_ATTACHMENT0};
        // .Invalidate(1, invalidateAttachments);
        app->commonRenderers.Blit2D(app->outputColor.Id());
    }

    {
        auto debugGroupGuard = gl::DebugGroupCtx("Debug pass");
        auto fbGuard         = gl::FramebufferCtx{0U, true};

        glm::mat4 model{1.0};
        model = glm::rotate(model, rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(1.6f, 0.0f, 0.0f));

        glm::mat4 mvp = camera * model;
        // app->commonRenderers.RenderBox(mvp, glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));

        float near = (std::sin(ctx.timeSec) + 1.5f) * 3.0f;
        gl::Frustum frustum{-0.3f, 1.3f + std::sin(2.0f * ctx.timeSec), -0.3f, 0.3f, near, 10.0f};
        // app->commonRenderers.RenderFrustum(mvp, frustum, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));

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

        app->commonRenderers.RenderAxes(mvp, 0.5f);

        gl::RenderVao(app->commonRenderers.VaoDatalessQuad(), GL_POINTS);
    }

    {
        auto debugGroupGuard = gl::DebugGroupCtx("Debug lines/points pass");
        auto fbGuard         = gl::FramebufferCtx{0U, true};
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
    auto& windowCtx     = engine::GetWindowContext(engine);
    GLFWwindow* window  = windowCtx.Window();
    auto oldCallbackEsc = windowCtx.SetKeyboardCallback(GLFW_KEY_ESCAPE, [=](bool pressed, bool released) {
        engine::QueueForNextFrame(
            engine, engine::UserActionType::WINDOW, [=](void*) { glfwSetWindowShouldClose(window, true); });
    });

    auto oldCallbackF = windowCtx.SetKeyboardCallback(GLFW_KEY_F, [=](bool pressed, bool released) {
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

    auto oldCallbackP = windowCtx.SetKeyboardCallback(GLFW_KEY_P, [=](bool pressed, bool released) {
        engine::QueueForNextFrame(engine, engine::UserActionType::RENDER, [=](void*) {
            static bool setToWireframe = true;
            if (!pressed) { return; }

            if (setToWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            XLOG("Wireframe mode: {}", setToWireframe);
            setToWireframe = !setToWireframe;
        });
    });

    auto oldCallbackLeft = windowCtx.SetMouseButtonCallback(GLFW_MOUSE_BUTTON_LEFT, [&](bool pressed, bool released) {
        if (released) {
            auto mousePosition = windowCtx.MousePosition();
            XLOG("LMB {} pos: {},{}", windowCtx.MouseInsideWindow(), mousePosition.x, mousePosition.y);
        }
    });
}

auto main() -> int {
    XLOG("! Compiled in DEBUG mode", 0);

    auto maybeEngine = engine::CreateEngine();
    if (maybeEngine == std::nullopt) {
        XLOGE("App failed to initialize the engine", 0);
        return -1;
    }

    auto* engine = *maybeEngine;
    ConfigureWindow(engine);

    {
        Application app;
        engine::SetApplicationData(engine, &app);

        auto _ = engine::SetRenderCallback(engine, Render);
        engine::BlockOnLoop(engine);
    }

    engine::DestroyEngine(engine);

    XLOG("App closed gracefully", 0);
    return 0;
}