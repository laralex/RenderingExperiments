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

   if (!gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress))){
      XLOG("Failed to initialize GLAD", 0);
      Terminate();
      return -1;
   }

   Terminate();
   return 0;
}