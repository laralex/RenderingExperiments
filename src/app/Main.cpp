#include <iostream>
#include <engine/GlHelpers.hpp>
#include <spdlog/spdlog.h>

int main() {
#ifdef XDEBUG
   std::cout << "!Compiled in DEBUG mode\n";
#endif
   XLOG("Hello world {}\n", 42);
   return 0;
}