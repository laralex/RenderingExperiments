#include <engine/Assets.hpp>
#include <engine/EngineLoop.hpp>
#include <engine/GlBuffer.hpp>
#include <engine/GlGuard.hpp>
#include <engine/GlProgram.hpp>
#include <engine/GlShader.hpp>
#include <engine/GlTextureUnits.hpp>
#include <engine/GlUniform.hpp>
#include <engine/GlVao.hpp>
#include <engine/Prelude.hpp>

#include <glm/mat4x4.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct Application final {
    engine::gl::GpuProgram program{};
    engine::gl::Vao vao{};
    engine::gl::GpuBuffer attributeBuffer{};
    engine::gl::GpuBuffer indexBuffer{};
    bool isInitialized = false;
};

constexpr float vertexData[] = {
    0.5f,  0.5f,  0.0f, // top right
    0.5f,  -0.5f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, // bottom left
    -0.5f, 0.5f,  0.0f  // top left
};

constexpr uint32_t indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

static void Render(engine::RenderCtx const& ctx, engine::WindowCtx const& windowCtx, void* appData) {
    using namespace engine;
    auto* app = static_cast<Application*>(appData);
    if (!app->isInitialized) {
        gl::InitializeOpenGl();

        constexpr static int32_t NUM_VDEFINES = 2;
        gl::ShaderDefine vdefines[NUM_VDEFINES] = {
            {.name="ATTRIB_POSITION_LOCATION", .value=0, .type=gl::ShaderDefine::INT32},
            {.name="UNIFORM_MVP_LOCATION", .value=10, .type=gl::ShaderDefine::INT32},
        };
        std::string vertexShaderCode = gl::LoadShaderCode("data/app/shaders/triangle.vert", vdefines, NUM_VDEFINES);

        constexpr static int32_t NUM_FDEFINES = 1;
        gl::ShaderDefine fdefines[NUM_FDEFINES] = {
            {.name="UNIFORM_COLOR_LOCATION", .value=0, .type=gl::ShaderDefine::INT32},
        };
        std::string fragmentShaderCode =  gl::LoadShaderCode("data/app/shaders/constant.frag", fdefines, NUM_FDEFINES);
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
        app->vao.LinkVertexAttribute(
            app->attributeBuffer,
            {.index           = 0,
             .valuesPerVertex = 3,
             .datatype        = GL_FLOAT,
             .normalized      = GL_FALSE,
             .stride          = 3 * sizeof(float),
             .offset          = 0});
        app->vao.LinkIndices(app->indexBuffer);

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

    gl::GlTextureUnits::BeginStateSnapshot();
    gl::GlTextureUnits::Bind2D(0U, 0U);
    gl::GlTextureUnits::Bind2D(1U, 0U);
    gl::GlTextureUnits::Bind2D(2U, 0U);
    gl::GlTextureUnits::BindCubemap(0U, 0U);
    gl::GlTextureUnits::BindCubemap(1U, 0U);
    gl::GlTextureUnits::BindCubemap(2U, 0U);
    gl::GlTextureUnits::EndStateSnapshot();

    gl::PushDebugGroup("Main pass");
    GLCALL(glClearColor(0.3f, 0.5f, 0.5f, 0.0f));
    {
        auto guard = gl::UniformCtx(app->program);
        glm::mat4 mvp = glm::mat4(1.0);
        gl::UniformMatrix4(/*location*/ 10, &mvp[0][0]);
        gl::UniformArray<4U>(/*location*/ 0, color, /*numValues*/ 1);
        gl::UniformValue(/*location*/ 0, color[0], color[1], color[2], color[3]);
    }
    GLCALL(glClear(GL_COLOR_BUFFER_BIT));
    GLCALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
    GLCALL(glBindVertexArray(app->vao.Id()));
    GLCALL(glUseProgram(app->program.Id()));
    GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
    gl::PopDebugGroup();
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