#include <engine/GlHelpers.hpp>
#include <engine/GlProgram.hpp>
#include <engine/RenderLoop.hpp>
#include <engine/Assets.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct Application final {
    GLuint program = GL_NONE;
    bool isInitialized = false;
};

static void Render(engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx, void* appData) {
    auto* app = static_cast<Application*>(appData);
    if (!app->isInitialized) {
        std::string vertexShaderCode = engine::LoadTextFile("data/app/shaders/triangle.vert");
        std::string fragmentShaderCode = engine::LoadTextFile("data/app/shaders/constant.frag");
        GLuint vertexShader   = engine::CompileShader(GL_VERTEX_SHADER, vertexShaderCode);
        GLuint fragmentShader = engine::CompileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        app->program = engine::CompileGraphicsProgram(vertexShader, fragmentShader);
        GLCALL(glDeleteShader(vertexShader));
        GLCALL(glDeleteShader(fragmentShader));

        app->isInitialized = true;
    }

    engine::f32 const red = 0.5f * (std::sin(ctx.timeSec) + 1.0f);
    GLCALL(glClearColor(red, 0.5f, 0.5f, 0.0f));
    GLCALL(glClear(GL_COLOR_BUFFER_BIT));
    GLCALL(glUseProgram(app->program));
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

    Application app;
    engine::SetApplicationData(&app);

    auto _ = engine::SetRenderCallback(Render);
    engine::BlockOnGameLoop(windowCtx);

    engine::Terminate();

    XLOG("App closed gracefully", 0);
    return 0;
}