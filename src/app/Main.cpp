#include <engine/GlHelpers.hpp>

#include <spdlog/spdlog.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

struct WindowCtx {
   engine::isize width;
   engine::isize height;

   WindowCtx(GLFWwindow* window) {
      int w, h;
      glfwGetFramebufferSize(window, &w, &h);
      width = w;
      height = h;
   }
};

struct RenderCtx final {
   engine::i64 timeNs{};
   engine::f32 timeSec{};
   engine::i64 prevTimeNs{};
   engine::f32 prevFrametimeMs{};
   engine::f32 prevFPS{};

   void Update(engine::i64 currentTimeNs, RenderCtx& destination) const {
      destination.timeNs = currentTimeNs;
      destination.timeSec = static_cast<engine::f32>(currentTimeNs / 1000) * 0.000001;
      destination.prevTimeNs = this->timeNs;
      engine::f32 frametimeMs = static_cast<engine::f32>(currentTimeNs - this->timeNs) * 0.000001;
      destination.prevFrametimeMs = frametimeMs;
      destination.prevFPS = 1000.0 / frametimeMs;
   }
};

static void GlfwErrorCallback(int errCode, char const* message) {
   XLOG("GLFW_ERROR({}): {}", errCode, message);
}

static void GlfwResizeCallback(GLFWwindow* window, int width, int height) {
   glViewport(0, 0, width, height);
   auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
   ctx->width = width;
   ctx->height = height;
}

static void Terminate() {
   glfwTerminate();
}

static void Render(RenderCtx const& ctx) {
   engine::f32 const red = 0.5f * (std::sin(ctx.timeSec) + 1.0f);
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

   GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
   if (window == nullptr) {
      XLOG("Failed to create GLFW window", 0)
      Terminate();
      return -1;
   }
   glfwMakeContextCurrent(window);
   glfwSetFramebufferSizeCallback(window, GlfwResizeCallback);
   glfwSwapInterval(1);
   glfwSetTime(0.0);

   if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))){
      XLOG("Failed to initialize GLAD", 0);
      Terminate();
      return -1;
   }

   WindowCtx windowCtx(window);
   glfwSetWindowUserPointer(window, &windowCtx);
   std::vector<RenderCtx> frameHistory(256);
   size_t frameIdx = 0, prevFrameHistoryIdx = 0;

   while (!glfwWindowShouldClose(window))
   {
      size_t const frameHistoryIdx = (prevFrameHistoryIdx + 1) % frameHistory.size();
      RenderCtx& renderCtx = frameHistory[frameHistoryIdx];
      RenderCtx const& prevRenderCtx = frameHistory[prevFrameHistoryIdx];
      prevRenderCtx.Update(glfwGetTimerValue(), renderCtx);

      Render(renderCtx);

      if (frameHistoryIdx == 0) {
         XLOG("{} FPS, {} ms", renderCtx.prevFPS, renderCtx.prevFrametimeMs);
      }

      glfwSwapBuffers(window);
      glfwPollEvents();

      prevFrameHistoryIdx = frameHistoryIdx;
      ++frameIdx;
   }

   Terminate();
   return 0;
}