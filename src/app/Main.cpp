#include <engine/Assets.hpp>
#include <engine/EngineLoop.hpp>
#include <engine/gl/Buffer.hpp>
#include <engine/gl/Framebuffer.hpp>
#include <engine/gl/Guard.hpp>
#include <engine/gl/Program.hpp>
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
    engine::gl::GpuProgram program{};
    engine::gl::Vao vao{};
    engine::gl::GpuBuffer attributeBuffer{};
    engine::gl::GpuBuffer indexBuffer{};
    engine::gl::Texture texture{};
    engine::gl::Framebuffer debugFramebuffer{};
    engine::gl::Texture debugDepth{};
    engine::gl::Framebuffer outputFramebuffer{};
    engine::gl::Texture outputColor{};
    engine::gl::Texture outputDepth{};

    bool isInitialized = false;
};

constexpr float vertexData[] = {
    0.5f,  0.5f,  0.0f, 0.0f, 0.0f, // top right
    0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, // bottom left
    -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, // top left
};

constexpr uint32_t indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
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

constexpr GLint ATTRIB_POSITION_LOCATION = 0;
constexpr GLint ATTRIB_UV_LOCATION       = 1;
constexpr GLint UNIFORM_TEXTURE_LOCATION = 0;
constexpr GLint UNIFORM_MVP_LOCATION     = 10;

static void Render(engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx, void* appData) {
    using namespace engine;
    auto* app = static_cast<Application*>(appData);
    if (!app->isInitialized) {
        glm::ivec2 screenSize = windowCtx.WindowSize();
        gl::InitializeOpenGl();

        constexpr static int32_t NUM_VDEFINES   = 3;
        gl::ShaderDefine vdefines[NUM_VDEFINES] = {
            {.name = "ATTRIB_POSITION_LOCATION", .value = ATTRIB_POSITION_LOCATION, .type = gl::ShaderDefine::INT32},
            {.name = "ATTRIB_UV_LOCATION", .value = ATTRIB_UV_LOCATION, .type = gl::ShaderDefine::INT32},
            {.name = "UNIFORM_MVP_LOCATION", .value = UNIFORM_MVP_LOCATION, .type = gl::ShaderDefine::INT32},
        };
        std::string vertexShaderCode = gl::LoadShaderCode("data/app/shaders/triangle.vert", vdefines, NUM_VDEFINES);

        constexpr static int32_t NUM_FDEFINES   = 1;
        gl::ShaderDefine fdefines[NUM_FDEFINES] = {
            {.name = "UNIFORM_TEXTURE_LOCATION", .value = UNIFORM_TEXTURE_LOCATION, .type = gl::ShaderDefine::INT32},
        };
        std::string fragmentShaderCode = gl::LoadShaderCode("data/app/shaders/texture.frag", fdefines, NUM_FDEFINES);
        GLuint vertexShader            = gl::CompileShader(GL_VERTEX_SHADER, vertexShaderCode);
        GLuint fragmentShader          = gl::CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        app->program                   = *gl::GpuProgram::Allocate(vertexShader, fragmentShader, "Test program");
        GLCALL(glDeleteShader(vertexShader));
        GLCALL(glDeleteShader(fragmentShader));

        app->attributeBuffer =
            gl::GpuBuffer::Allocate(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertexData, sizeof(vertexData), "Test VBO");
        app->indexBuffer =
            gl::GpuBuffer::Allocate(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices), "Test EBO");
        app->vao = gl::Vao::Allocate("Test VAO");
        (void)gl::VaoCtx{app->vao}
            .LinkVertexAttribute(
                app->attributeBuffer,
                {.index           = ATTRIB_POSITION_LOCATION,
                 .valuesPerVertex = 3,
                 .datatype        = GL_FLOAT,
                 .stride          = 5 * sizeof(float),
                 .offset          = 0})
            .LinkVertexAttribute(
                app->attributeBuffer,
                {.index           = ATTRIB_UV_LOCATION,
                 .valuesPerVertex = 2,
                 .datatype        = GL_FLOAT,
                 .stride          = 5 * sizeof(float),
                 .offset          = 3 * sizeof(float)})
            .LinkIndices(app->indexBuffer);

        app->texture = gl::Texture::Allocate2D(GL_TEXTURE_2D, glm::ivec3(4, 2, 0), GL_RGB8, "Test texture");
        (void)gl::TextureCtx{app->texture}
            .Fill2D(GL_RGB, GL_UNSIGNED_BYTE, textureData, app->texture.Size())
            .GenerateMipmaps();

        app->outputColor =
            gl::Texture::Allocate2D(GL_TEXTURE_2D, glm::ivec3(screenSize.x, screenSize.y, 0), GL_RGBA8, "Output color");
        app->outputDepth = gl::Texture::Allocate2D(
            GL_TEXTURE_2D, glm::ivec3(screenSize.x, screenSize.y, 0), GL_DEPTH24_STENCIL8, "Output depth");
        app->outputFramebuffer = gl::Framebuffer::Allocate("Main Pass FBO");
        (void)gl::FramebufferCtx{app->outputFramebuffer, true}
            .LinkTexture(GL_COLOR_ATTACHMENT0, app->outputColor)
            .LinkTexture(GL_DEPTH_STENCIL_ATTACHMENT, app->outputDepth);

        app->debugDepth = gl::Texture::Allocate2D(
            GL_TEXTURE_2D, glm::ivec3(screenSize.x, screenSize.y, 0), GL_DEPTH24_STENCIL8, "Debug depth");
        app->debugFramebuffer = gl::Framebuffer::Allocate("Debug Pass FBO");
        (void)gl::FramebufferCtx{app->debugFramebuffer, true}
            .LinkTexture(GL_COLOR_ATTACHMENT0, app->outputColor)
            .LinkTexture(GL_DEPTH_STENCIL_ATTACHMENT, app->debugDepth);
        app->isInitialized = true;
    }

    GLfloat const color[]{0.5f * (std::sin(ctx.timeSec) + 1.0f), 0.0f, 0.0f, 1.0f};
    gl::GlGuardAux guardBind;
    gl::GlGuardVertex guardVert(true);
    gl::GlGuardFlags guardF;
    gl::GlGuardDepth guardD(true);
    gl::GlGuardStencil guardS;
    gl::GlGuardBlend guardB(true);
    gl::GlGuardViewport guardV(true);
    gl::GlGuardColor guardR;
    gl::GlGuardFramebuffer guardFb(true);

    gl::GlTextureUnits::BeginStateSnapshot();
    gl::GlTextureUnits::Bind2D(0U, 0U);
    gl::GlTextureUnits::Bind2D(1U, 0U);
    gl::GlTextureUnits::Bind2D(2U, 0U);
    gl::GlTextureUnits::BindCubemap(0U, 0U);
    gl::GlTextureUnits::BindCubemap(1U, 0U);
    gl::GlTextureUnits::BindCubemap(2U, 0U);
    gl::GlTextureUnits::EndStateSnapshot();

    glm::vec3 cameraPosition = glm::vec3(0.0f, 2.0f, std::sin(ctx.timeSec) - 1.5f);
    glm::vec3 cameraTarget   = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp       = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::ivec2 renderSize    = app->outputColor.Size();
    glm::ivec2 screenSize    = windowCtx.WindowSize();
    float aspectRatio        = static_cast<float>(screenSize.x) / static_cast<float>(screenSize.y);
    glm::mat4 proj           = glm::perspective(glm::radians(60.0f), aspectRatio, 0.001f, 10.0f);
    glm::mat4 view           = glm::lookAtRH(cameraPosition, cameraTarget, cameraUp);
    glm::mat4 camera         = proj * view;
    XLOG("screen {} {}", renderSize.x, renderSize.y);

    {
        auto debugGroupGuard = gl::DebugGroupCtx("Main pass");
        glViewport(0, 0, renderSize.x, renderSize.y);
        auto fbGuard         = gl::FramebufferCtx{app->outputFramebuffer, true};
        fbGuard              = fbGuard.ClearColor(0, 0.1f, 0.2f, 0.3f, 0.0f).ClearDepthStencil(1.0f, 0);

        auto programGuard            = gl::UniformCtx(app->program);
        glm::mat4 model              = glm::rotate(glm::mat4(1.0), ctx.timeSec * 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 mvp                = camera * model;
        constexpr GLint TEXTURE_SLOT = 0;
        gl::UniformTexture(UNIFORM_TEXTURE_LOCATION, TEXTURE_SLOT);
        gl::UniformMatrix4(UNIFORM_MVP_LOCATION, &mvp[0][0]);
        gl::GlTextureUnits::Bind2D(TEXTURE_SLOT, app->texture.Id());
        gl::GlTextureUnits::Bind2D(TEXTURE_SLOT, gl::CommonRenderers::TextureStubColor().Id());
        if (windowCtx.MouseInsideWindow()) {
            gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, gl::CommonRenderers::SamplerNearest().Id());
        } else {
            gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, gl::CommonRenderers::SamplerLinearMips().Id());
        }
        GLCALL(glEnable(GL_CULL_FACE));
        GLCALL(glEnable(GL_DEPTH_TEST));
        GLCALL(glDepthMask(GL_TRUE));
        GLCALL(glFrontFace(GL_CCW));
        GLCALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
        GLCALL(glBindVertexArray(app->vao.Id()));
        GLCALL(glUseProgram(app->program.Id()));
        GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

        gl::GlTextureUnits::BindSampler(TEXTURE_SLOT, 0);
    }

    {
        auto debugGroupGuard = gl::DebugGroupCtx("Debug pass");
        // auto fbGuard         = gl::FramebufferCtx{0U, true};
        auto fbGuard = gl::FramebufferCtx{app->debugFramebuffer, true};
        fbGuard      = fbGuard.ClearDepth(1.0f);
        glm::mat4 model = glm::rotate(glm::mat4(1.0), ctx.timeSec * 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 mvp   = camera * model;
        gl::CommonRenderers::RenderAxes(mvp);
    }

    {
        // present
        glViewport(0, 0, screenSize.x, screenSize.y);
        gl::CommonRenderers::Blit2D(app->outputColor.Id(), 0U);
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