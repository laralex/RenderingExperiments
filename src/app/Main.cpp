#include <engine/Assets.hpp>
#include <engine/EngineLoop.hpp>
#include <engine/gl/Buffer.hpp>
#include <engine/gl/CommonRenderers.hpp>
#include <engine/gl/Framebuffer.hpp>
#include <engine/gl/FlatRenderer.hpp>
#include <engine/gl/Guard.hpp>
#include <engine/gl/Init.hpp>
#include <engine/gl/Program.hpp>
#include <engine/gl/Renderbuffer.hpp>
#include <engine/gl/Sampler.hpp>
#include <engine/gl/Shader.hpp>
#include <engine/gl/Texture.hpp>
#include <engine/gl/TextureUnits.hpp>
#include <engine/gl/Uniform.hpp>
#include <engine/gl/Vao.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct Application final {
    ~Application() {
        XLOG("Disposing application", 0);
        engine::gl::DisposeOpenGl();
    }

    engine::gl::GpuProgram program{};
    engine::gl::Vao vao{};
    engine::gl::GpuBuffer attributeBuffer{};
    engine::gl::GpuBuffer positionBuffer{};
    engine::gl::GpuBuffer indexBuffer{};
    engine::gl::Texture texture{};
    engine::gl::Framebuffer outputFramebuffer{};
    engine::gl::Texture outputColor{};
    engine::gl::Texture outputDepth{};
    engine::gl::Renderbuffer renderbuffer{};
    engine::gl::CommonRenderers commonRenderers{};
    engine::gl::FlatRenderer flatRenderer{};

    bool isInitialized = false;
};

struct Vertex {
    glm::vec2 uv;
    glm::vec3 normal;
};

constexpr GLfloat vertexPositions[] = {
    -0.5f, -0.5f, 0.5f, // 0
    0.5f, -0.5f, 0.5f, // 1
    0.5f, 0.5f, 0.5f, // 2
    -0.5f, 0.5f, 0.5f, // 3
    -0.5f, 0.5f, -0.5f, // 4
    0.5f, 0.5f, -0.5f, // 5
    0.5f, -0.5f, -0.5f, // 6
    -0.5f, -0.5f, -0.5f, // 7

    -0.5f, -0.5f, 0.5f, // 8 (dup 0-7)
    0.5f, -0.5f, 0.5f, // 9
    0.5f, 0.5f, 0.5f, // 10
    -0.5f, 0.5f, 0.5f, // 11
    -0.5f, 0.5f, -0.5f, // 12
    0.5f, 0.5f, -0.5f, // 13
    0.5f, -0.5f, -0.5f, // 14
    -0.5f, -0.5f, -0.5f, // 15

    -0.5f, -0.5f, 0.5f, // 16 (dup 0-7)
    0.5f, -0.5f, 0.5f, // 17
    0.5f, 0.5f, 0.5f, // 18
    -0.5f, 0.5f, 0.5f, // 19
    -0.5f, 0.5f, -0.5f, // 20
    0.5f, 0.5f, -0.5f, // 21
    0.5f, -0.5f, -0.5f, // 22
    -0.5f, -0.5f, -0.5f, // 23
};

constexpr Vertex vertexData[] = {
    { { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }, // 0
    { { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }, // 1
    { { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } }, // 2
    { { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } }, // 3
    { { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } }, // 4
    { { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } }, // 5
    { { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } }, // 6
    { { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } }, // 7
    { { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } }, // 8
    { { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } }, // 9
    { { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } }, // 10
    { { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } }, // 11
    { { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } }, // 12
    { { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } }, // 13
    { { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } }, // 14
    { { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } }, // 15
    { { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } }, // 16
    { { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } }, // 17
    { { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } }, // 18
    { { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } }, // 19
    { { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } }, // 20
    { { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } }, // 21
    { { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } }, // 22
    { { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } }, // 23
};

constexpr uint32_t Z_POS0 = 0, Z_POS1 = 1, Z_POS2 = 2, Z_POS3 = 3;
constexpr uint32_t Z_NEG0 = 4, Z_NEG1 = 5, Z_NEG2 = 6, Z_NEG3 = 7;
constexpr uint32_t X_POS0 = 10, X_POS1 = 9, X_POS2 = 14, X_POS3 = 13;
constexpr uint32_t X_NEG0 = 11, X_NEG1 = 12, X_NEG2 = 15, X_NEG3 = 8;
constexpr uint32_t Y_POS0 = 18, Y_POS1 = 21, Y_POS2 = 20, Y_POS3 = 19;
constexpr uint32_t Y_NEG0 = 23, Y_NEG1 = 22, Y_NEG2 = 17, Y_NEG3 = 16;

constexpr uint32_t indices[] = {
    Z_POS0, Z_POS1, Z_POS2,
    Z_POS0, Z_POS2, Z_POS3,
    Z_NEG0, Z_NEG1, Z_NEG2,
    Z_NEG0, Z_NEG2, Z_NEG3,

    X_POS0, X_POS1, X_POS2,
    X_POS0, X_POS2, X_POS3,
    X_NEG0, X_NEG1, X_NEG2,
    X_NEG0, X_NEG2, X_NEG3,

    Y_POS0, Y_POS1, Y_POS2,
    Y_POS0, Y_POS2, Y_POS3,
    Y_NEG0, Y_NEG1, Y_NEG2,
    Y_NEG0, Y_NEG2, Y_NEG3,
};

constexpr uint8_t textureData[] = {
    20,  20,  20,  // 00
    40,  40,  40,  // 01
    60,  60,  60,  // 02
    80,  80,  80,  // 03
    100, 100, 100, // 10
    120, 120, 120, // 11
    140, 140, 140, // 12
    160, 160, 160, // 13
};

constexpr GLint ATTRIB_POSITION_LOCATION           = 0;
constexpr GLint ATTRIB_UV_LOCATION                 = 1;
constexpr GLint ATTRIB_NORMAL_LOCATION             = 2;
constexpr GLint UNIFORM_TEXTURE_LOCATION           = 0;
constexpr GLint UNIFORM_MVP_LOCATION               = 10;
constexpr glm::ivec2 INTERMEDITE_RENDER_RESOLUTION = glm::ivec2(1600, 900);

static void InitializeApplication(engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx, Application* app) {
    using namespace engine;
    glm::ivec2 screenSize = INTERMEDITE_RENDER_RESOLUTION;
    gl::InitializeOpenGl();
    app->commonRenderers.Initialize();

    gl::ShaderDefine const defines[] = {
        {.name = "ATTRIB_POSITION_LOCATION", .value = ATTRIB_POSITION_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "ATTRIB_UV_LOCATION", .value = ATTRIB_UV_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_MVP_LOCATION", .value = UNIFORM_MVP_LOCATION, .type = gl::ShaderDefine::INT32},
        {.name = "UNIFORM_TEXTURE_LOCATION", .value = UNIFORM_TEXTURE_LOCATION, .type = gl::ShaderDefine::INT32},
    };

    auto maybeProgram = gl::LinkProgramFromFiles(
        "data/app/shaders/triangle.vert", "data/app/shaders/texture.frag", CpuView{defines, std::size(defines)},
        "Test program");
    assert(maybeProgram);
    app->program = std::move(*maybeProgram);

    app->positionBuffer =
        gl::GpuBuffer::Allocate(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertexPositions, sizeof(vertexPositions), "Test positions VBO");
    app->attributeBuffer =
        gl::GpuBuffer::Allocate(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertexData, sizeof(vertexData), "Test attributes VBO");
    app->indexBuffer =
        gl::GpuBuffer::Allocate(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices), "Test EBO");
    app->vao = gl::Vao::Allocate("Test VAO");
    (void)gl::VaoMutableCtx{app->vao}
        .MakeVertexAttribute(
            app->positionBuffer,
            {.index           = ATTRIB_POSITION_LOCATION,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(glm::vec3),
             .offset          = 0})
        .MakeVertexAttribute(
            app->attributeBuffer,
            {.index           = ATTRIB_UV_LOCATION,
             .valuesPerVertex = 2,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(Vertex),
             .offset          = offsetof(Vertex, uv)})
        .MakeVertexAttribute(
            app->attributeBuffer,
            {.index           = ATTRIB_NORMAL_LOCATION,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .stride          = sizeof(Vertex),
             .offset          = offsetof(Vertex, normal)})
        .MakeIndexed(app->indexBuffer, GL_UNSIGNED_INT);

    app->texture = gl::Texture::Allocate2D(GL_TEXTURE_2D, glm::ivec3(4, 2, 0), GL_RGB8, "Test texture");
    (void)gl::TextureCtx{app->texture}
        .Fill2D(GL_RGB, GL_UNSIGNED_BYTE, textureData, app->texture.Size())
        .GenerateMipmaps();

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

    app->flatRenderer = gl::AllocateFlatRenderer();
}
static void Render(engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx, void* appData) {
    using namespace engine;
    auto* app = static_cast<Application*>(appData);
    if (!app->isInitialized) {
        InitializeApplication(ctx, windowCtx, app);
        app->isInitialized = true;
    }

    float cameraZ            = std::sin(ctx.timeSec) - 1.5f;
    glm::vec3 cameraPosition = glm::vec3(0.0f, 2.0f, cameraZ * 0.0f - 1.5f);
    glm::vec3 cameraTarget   = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp       = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::ivec2 renderSize    = app->outputColor.Size();
    glm::ivec2 screenSize    = windowCtx.WindowSize();
    float aspectRatio        = static_cast<float>(screenSize.x) / static_cast<float>(screenSize.y);
    glm::mat4 proj           = glm::perspective(glm::radians(30.0f), aspectRatio, 0.1f, 100.0f);
    glm::mat4 view           = glm::lookAtRH(cameraPosition, cameraTarget, cameraUp);
    glm::mat4 camera         = proj * view;

    float rotationSpeed = ctx.timeSec * 0.5f;
    {
        glm::mat4 model = glm::mat4(1.0f);
        model           = glm::rotate(model, rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
        model           = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.001f));
        model           = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

        auto debugGroupGuard = gl::DebugGroupCtx("Main pass");
        glViewport(0, 0, renderSize.x, renderSize.y);
        auto fbGuard = gl::FramebufferCtx{app->outputFramebuffer, true};
        fbGuard      = fbGuard.ClearColor(0, 0.1f, 0.2f, 0.3f, 0.0f).ClearDepthStencil(1.0f, 0);

        auto programGuard = gl::UniformCtx(app->program);

        glm::mat4 mvp                = camera * model;
        constexpr GLint TEXTURE_SLOT = 0;
        programGuard.SetUniformTexture(UNIFORM_TEXTURE_LOCATION, TEXTURE_SLOT);
        programGuard.SetUniformMatrix4(UNIFORM_MVP_LOCATION, glm::value_ptr(mvp));
        gl::GlTextureUnits::Bind2D(TEXTURE_SLOT, app->texture.Id());
        // gl::GlTextureUnits::Bind2D(TEXTURE_SLOT, app->commonRenderers.TextureStubColor().Id());
        if (windowCtx.MouseInsideWindow()) {
            gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, app->commonRenderers.SamplerNearest().Id());
        } else {
            gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, app->commonRenderers.SamplerLinearMips().Id());
        }
        GLCALL(glEnable(GL_CULL_FACE));
        GLCALL(glEnable(GL_DEPTH_TEST));
        GLCALL(glDepthMask(GL_TRUE));
        GLCALL(glFrontFace(GL_CCW));
        GLCALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
        GLCALL(glUseProgram(app->program.Id()));
        gl::RenderVao(app->vao);

        gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, 0);
    }

    {
        glViewport(0, 0, renderSize.x, renderSize.y);
        auto fbGuard = gl::FramebufferCtx{app->outputFramebuffer, true};

        glm::mat4 model = glm::mat4(1.0f);
        model           = glm::rotate(model, rotationSpeed, glm::vec3(0.5f, 0.2f, 1.0f));
        model           = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        model           = glm::translate(model, glm::vec3(-2.0f-std::sin(ctx.timeSec), 0.0f, 0.0f));
        glm::mat4 mvp                = camera * model;

        glm::vec3 lightPosition{0.0f, 1.0f*std::sin(ctx.timeSec), 0.0f};
        glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), lightPosition);
        lightModel = glm::scale(lightModel, glm::vec3(0.1f, 0.1f, 0.1f));

        app->commonRenderers.RenderAxes(mvp);
        app->commonRenderers.RenderAxes(camera * lightModel);

        GLCALL(glEnable(GL_CULL_FACE));
        GLCALL(glEnable(GL_DEPTH_TEST));
        GLCALL(glDepthMask(GL_TRUE));
        GLCALL(glDepthFunc(GL_LEQUAL));
        gl::RenderFlatMesh(app->flatRenderer, app->vao, GL_TRIANGLES, model, camera, lightPosition);
    }


    {
        // present
        glViewport(0, 0, screenSize.x, screenSize.y);
        GLenum invalidateAttachments[1] = {GL_COLOR_ATTACHMENT0};
        auto dstGuard                   = gl::FramebufferCtx{0U, true}.ClearDepthStencil(1.0f, 0);
        // .Invalidate(1, invalidateAttachments);
        app->commonRenderers.Blit2D(app->outputColor.Id());
    }

    {
        auto debugGroupGuard = gl::DebugGroupCtx("Debug pass");
        auto fbGuard         = gl::FramebufferCtx{0U, true};

        glm::mat4 model{1.0};
        model           = glm::rotate(model, rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
        model           = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 mvp = camera * model;
        // app->commonRenderers.RenderBox(mvp, glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));

        float near = (std::sin(ctx.timeSec) + 1.5f) * 3.0f;
        gl::Frustum frustum{-0.3f, 1.3f + std::sin(2.0f * ctx.timeSec), -0.3f, 0.3f, near, 10.0f};
        // app->commonRenderers.RenderFrustum(mvp, frustum, glm::vec4(0.0f, 0.5f, 1.0f, 1.0f));

        {
            // glm::mat4 mvp = camera * model;
            glm::vec2 billboardSize = glm::vec2{1.0f, 1.0f};
            glm::vec3 billboardPivotOffset = glm::vec3{0.0f, 0.0f, 0.0f};
            gl::ScreenShaderArgs screen {
                .pixelsPerUnitX = 0.001f * static_cast<float>(screenSize.x),
                .pixelsPerUnitY = 0.001f * static_cast<float>(screenSize.y),
                .pixelsHeight = static_cast<float>(screenSize.y),
                .aspectRatio = aspectRatio
            };
            app->commonRenderers.RenderBillboard(gl::BillboardRenderArgs{
                app->commonRenderers.VaoDatalessQuad(), GL_TRIANGLE_STRIP,
                screen, mvp, billboardSize, billboardPivotOffset,
            });
        }

        app->commonRenderers.RenderAxes(mvp);

        gl::RenderVao(app->commonRenderers.VaoDatalessQuad(), GL_POINTS);
    }

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