#include <engine/GlHelpers.hpp>
#include <engine/RenderLoop.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

static void Render(engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx) {
    engine::f32 const red = 0.5f * (std::sin(ctx.timeSec) + 1.0f);
    glClearColor(red, 0.5f, 0.5f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

auto main() -> int {
    XLOG("! Compiled in DEBUG mode", 0);

    GLFWwindow* window = engine::Initialize();

    engine::WindowCtx windowCtx(window);
    windowCtx.SetKeyboardCallback(
        GLFW_KEY_ESCAPE, [=](bool pressed, bool released) { glfwSetWindowShouldClose(window, true); });

    windowCtx.SetKeyboardCallback(GLFW_KEY_F, [=](bool pressed, bool released) {
        static bool setToFullscreen = true;
        if (!pressed) { return; }

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
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

    auto _ = engine::SetRenderCallback(Render);
    engine::BlockOnGameLoop(window, windowCtx);
    engine::Terminate();

    XLOG("App closed gracefully", 0);
    return 0;
}