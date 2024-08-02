#include <iostream>
#include <engine/GlHelpers.hpp>
#include <spdlog/spdlog.h>

int main() {
#ifdef NDEBUG
   std::cout << "!Compiled with NDEBUG\n";
#endif
   XLOG("Hello world {}\n", 42);
   return 0;
}