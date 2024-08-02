#include <iostream>
#include <engine/GlHelpers.hpp>
#include <spdlog/spdlog.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>


static void GlfwErrorCallback(int errCode, char const* message) {
   XLOG("GLFW_ERROR({}): {}", errCode, message);
}

static void Terminate() {
   glfwTerminate();
}

struct RenderCtx {
   engine::isize width{};
   engine::isize height{};
   engine::i64 timeNs{};
   engine::f32 timeSec{};

   void Update() {
      timeSec = static_cast<engine::f32>(timeNs / 1000) * 0.000001;
   }
};

static void Render(RenderCtx const& ctx) {
   glViewport(0, 0, ctx.width, ctx.height);
   engine::f32 red = 0.5f * (std::sin(ctx.timeSec) + 1.0f);
   XLOG("{} {}", red, ctx.timeSec);
   glClearColor(red, 0.5f, 0.5f, 0.0f);
   glClear(GL_COLOR_BUFFER_BIT);
}

int main() {
#ifdef XDEBUG
   std::cout << "!Compiled in DEBUG mode\n";
#endif
   XLOG("Hello world {}\n", engine::Add(20.00022f, 22.00021f));

   glfwSetErrorCallback(GlfwErrorCallback);
   if (!glfwInit()) {
      XLOG("Failed to initialize GLFW", 0)
      Terminate();
      return -1;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   engine::isize winWidth = 800, winHeight = 600;
   GLFWwindow* window = glfwCreateWindow(winWidth, winHeight, "LearnOpenGL", NULL, NULL);
   if (window == nullptr) {
      XLOG("Failed to create GLFW window", 0)
      Terminate();
      return -1;
   }
   glfwMakeContextCurrent(window);
   glfwSwapInterval(1);

   if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))){
      XLOG("Failed to initialize GLAD", 0);
      Terminate();
      return -1;
   }

   RenderCtx renderCtx{};
   {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      renderCtx.width = width;
      renderCtx.height = height;
   }

   renderCtx.timeNs = 0;
   while (!glfwWindowShouldClose(window))
   {
      renderCtx.timeNs = glfwGetTimerValue();
      renderCtx.Update();
      Render(renderCtx);

      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   Terminate();
   return 0;
}