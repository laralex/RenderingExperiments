#include <engine/GlHelpers.hpp>

#include <spdlog/spdlog.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <unordered_map>



using ButtonCallback = std::function<void(bool, bool)>;
using AxisCallback = std::function<void(engine::f32)>;

// forward declaration required to preserve internal linkage
static void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

class WindowCtx final {

public:

#define Self WindowCtx
   explicit Self(GLFWwindow* window);
   ~Self() = default;
   Self(Self const&) = delete;
   Self(Self &&) = default;
   void operator=(Self const&) = delete;
   void operator=(Self &&) = delete;
#undef Self

   using GlfwKey = int;
   auto SetKeyboardCallback(GlfwKey keyboardKey, ButtonCallback callback) -> ButtonCallback;
   void SetResolution(engine::isize width, engine::isize height);

private:
   engine::isize width_{0};
   engine::isize height_{0};

   std::unordered_map<GlfwKey, ButtonCallback> buttons_;

   friend void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

WindowCtx::WindowCtx(GLFWwindow* window) : buttons_() {
   int w, h;
   glfwGetFramebufferSize(window, &w, &h);
   width_ = w;
   height_ = h;
}

auto WindowCtx::SetKeyboardCallback(GlfwKey keyboardKey, ButtonCallback callback) -> ButtonCallback {
   auto found = buttons_.find(keyboardKey);
   ButtonCallback oldCallback = nullptr;
   if (found != buttons_.end()) {
      oldCallback = found->second;
   }
   buttons_[keyboardKey] = callback;
   return oldCallback;
}

void WindowCtx::SetResolution(engine::isize width, engine::isize height) {
   width_ = width;
   if (width_ < 0) { width_ = 0; }
   height_ = height;
   if (height_ < 0) { height_ = 0; }
}

struct RenderCtx final {
   engine::i64 timeNs{0};
   engine::f32 timeSec{0.0f};
   engine::i64 prevTimeNs{0};
   engine::f32 prevFrametimeMs{0.0f};
   engine::f32 prevFPS{0.0f};

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
   XLOGE("GLFW_ERROR({}): {}", errCode, message);
}

static void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
   if (ctx == nullptr) { return; }
   if (auto const found = ctx->buttons_.find(key); found != ctx->buttons_.end()) {
      found->second(action == GLFW_PRESS, action == GLFW_RELEASE);
   }
}

static void GlfwResizeCallback(GLFWwindow* window, int width, int height) {
   auto ctx = static_cast<WindowCtx*>(glfwGetWindowUserPointer(window));
   if (ctx == nullptr) { return; }
   glViewport(0, 0, width, height);
   ctx->SetResolution(width, height);
}

static void Terminate() {
   glfwTerminate();
}

static void Render(RenderCtx const& ctx, WindowCtx const& windowCtx) {
   engine::f32 const red = 0.5f * (std::sin(ctx.timeSec) + 1.0f);
   glClearColor(red, 0.5f, 0.5f, 0.0f);
   glClear(GL_COLOR_BUFFER_BIT);
}

static auto CreateWindow(int width, int height) -> GLFWwindow* {
   glfwSetErrorCallback(GlfwErrorCallback);
   if (!glfwInit()) {
      XLOGE("Failed to initialize GLFW", 0)
      return nullptr;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
   glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

   GLFWmonitor* monitor = glfwGetPrimaryMonitor();
   GLFWvidmode const* mode = glfwGetVideoMode(monitor);

   glfwWindowHint(GLFW_RED_BITS, mode->redBits);
   glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
   glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
   glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

   GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", nullptr, nullptr);
   if (window == nullptr) {
      XLOGE("Failed to create GLFW window", 0)
      return nullptr;
   }

   glfwMakeContextCurrent(window);
   if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))){
      XLOGE("Failed to initialize GLAD", 0);
      return nullptr;
   }

   glfwSetFramebufferSizeCallback(window, GlfwResizeCallback);
   glfwSetKeyCallback(window, GlfwKeyCallback);
   glfwSwapInterval(1);
   glfwSetTime(0.0);

   return window;
}

static auto UpdateRenderLoop(std::vector<RenderCtx>& frameHistory, size_t frameIdx) -> RenderCtx& {
   size_t const frameHistoryIdx = frameIdx % frameHistory.size();
   size_t const prevFrameHistoryIdx = (frameIdx - 1) % frameHistory.size();

   RenderCtx& renderCtx = frameHistory[frameHistoryIdx];
   RenderCtx const& prevRenderCtx = frameHistory[prevFrameHistoryIdx];
   prevRenderCtx.Update(glfwGetTimerValue(), renderCtx);

   if (frameHistoryIdx == 0) {
      XLOG("{} FPS, {} ms", renderCtx.prevFPS, renderCtx.prevFrametimeMs);
   }
   return renderCtx;
}

auto main() -> int {
#ifdef XDEBUG
   std::cout << "! Compiled in DEBUG mode\n";
#endif

   GLFWwindow* window = CreateWindow(800, 600);
   if (window == nullptr) {
      Terminate();
      return -1;
   }

   WindowCtx windowCtx(window);
   glfwSetWindowUserPointer(window, &windowCtx);

   windowCtx.SetKeyboardCallback(GLFW_KEY_ESCAPE, [=](bool pressed, bool released){
      glfwSetWindowShouldClose(window, true);
   });

   windowCtx.SetKeyboardCallback(GLFW_KEY_F, [=](bool pressed, bool released){
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
      setToFullscreen = !setToFullscreen;
   });

   std::vector<RenderCtx> frameHistory(256);
   size_t frameIdx = 0;

   while (!glfwWindowShouldClose(window))
   {
      RenderCtx& renderCtx = UpdateRenderLoop(frameHistory, frameIdx);
      Render(renderCtx, windowCtx);

      glfwSwapBuffers(window);
      glfwPollEvents();

      ++frameIdx;
   }

   XLOG("Window is closed gracefully", 0);
   Terminate();
   return 0;
}