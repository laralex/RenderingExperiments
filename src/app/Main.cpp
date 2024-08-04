#include <engine/Assets.hpp>
#include <engine/GlBuffer.hpp>
#include <engine/GlCapabilities.hpp>
#include <engine/GlHelpers.hpp>
#include <engine/GlProgram.hpp>
#include <engine/GlGuard.hpp>
#include <engine/GlVao.hpp>
#include <engine/RenderLoop.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct Application final {
    GLuint program = GL_NONE;
    engine::gl::Vao vao{};
    engine::gl::GpuBuffer attributeBuffer{};
    engine::gl::GpuBuffer indexBuffer{};
    bool isInitialized = false;
};

constexpr engine::f32 vertexData[] = {
    0.5f,  0.5f,  0.0f, // top right
    0.5f,  -0.5f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, // bottom left
    -0.5f, 0.5f,  0.0f  // top left
};

constexpr engine::u32 indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

static void Render(engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx, void* appData) {
    using namespace engine;
    auto* app = static_cast<Application*>(appData);
    if (!app->isInitialized) {
        gl::GlCapabilities::Initialize();

        std::string vertexShaderCode   = LoadTextFile("data/app/shaders/triangle.vert");
        std::string fragmentShaderCode = LoadTextFile("data/app/shaders/constant.frag");
        GLuint vertexShader            = CompileShader(GL_VERTEX_SHADER, vertexShaderCode);
        GLuint fragmentShader          = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        app->program                   = CompileGraphicsProgram(vertexShader, fragmentShader);
        GLCALL(glDeleteShader(vertexShader));
        GLCALL(glDeleteShader(fragmentShader));

        app->attributeBuffer.Initialize(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertexData, sizeof(vertexData));
        app->indexBuffer.Initialize(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices));
        app->vao.Initialize();
        app->vao.DefineVertexAttribute(
            app->attributeBuffer,
            {.index           = 0,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .normalized      = GL_FALSE,
             .stride          = 3 * sizeof(engine::f32),
             .offset          = 0});
        app->vao.DefineIndices(app->indexBuffer);

        app->isInitialized = true;
    }

    engine::f32 const red = 0.5f * (std::sin(ctx.timeSec) + 1.0f);
    gl::GlGuardBindings guardBind;
    gl::GlGuardVertex guardVert(true);
    gl::GlGuardFlags guardF;
    gl::GlGuardDepth guardD(true);
    gl::GlGuardStencil guardS;
    gl::GlGuardBlend guardB(true);
    gl::GlGuardViewport guardV(true);
    gl::GlGuardColor guardR;
    GLCALL(glClearColor(red, 0.5f, 0.5f, 0.0f));
    GLCALL(glClear(GL_COLOR_BUFFER_BIT));
    GLCALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
    GLCALL(glBindVertexArray(app->vao.Id()));
    GLCALL(glUseProgram(app->program));
    GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
}

static auto ConfigureWindow(engine::WindowCtx& windowCtx) {
    GLFWwindow* window = windowCtx.Window();
    windowCtx.SetKeyboardCallback(
        GLFW_KEY_ESCAPE, [=](bool pressed, bool released) { glfwSetWindowShouldClose(window, true); });

    windowCtx.SetKeyboardCallback(GLFW_KEY_F, [=](bool pressed, bool released) {
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

    windowCtx.SetKeyboardCallback(GLFW_KEY_P, [=](bool pressed, bool released) {
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

    windowCtx.SetMouseButtonCallback(GLFW_MOUSE_BUTTON_LEFT, [&](bool pressed, bool released) {
        if (released) {
            auto mousePosition = windowCtx.MousePosition();
            XLOG("LMB {} pos: {},{}", windowCtx.MouseInsideWindow(), mousePosition.x, mousePosition.y);
        }
    });
}

auto main() -> int {
    XLOG("! Compiled in DEBUG mode", 0);

    auto& windowCtx = engine::Initialize();
    if (!windowCtx.IsInitialized()) {
        XLOGE("App failed to initialize the engine", 0);
        return -1;
    }
    ConfigureWindow(windowCtx);

    {
        Application app;
        engine::SetApplicationData(&app);

        auto _ = engine::SetRenderCallback(Render);
        engine::BlockOnGameLoop(windowCtx);
    }

    engine::Terminate();

    XLOG("App closed gracefully", 0);
    return 0;
}