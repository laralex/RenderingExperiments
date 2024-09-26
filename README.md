## C++/OpenGL rendering engine

Originated from the famous [Learnopengl.com](https://learnopengl.com/) tutorial.

Now it's a playground for my simple graphical experiments - more concerned about rendering algorithms and scientific papers, less concerned about maximizing performance (though within realtime)

Features:
- shader code generation (`#include`, `#define`)
- code can be recompiled without stopping the application (hot-reloading)
- the project is split into:
    * engine - contains the main loop, window management, rendering wrappers and utilities
    * application - instantiates an engine, provides window callbacks, provides the rendering callback, instantiates and uses engine utilities


## Installing prerequisites:
The project was developed and tested on Ubuntu 20.04 (Wayland).

First of all you need [GNU Make](https://www.gnu.org/software/make/) (tested version `4.2.1`)
> Try to run `make -version` in console. If it's not already installed, run
> ```bash
> sudo apt update
> sudo apt install make
> ```

Now you can choose to install prerequisites automatically, or manually

* Automatically: it will execute `sudo` commands, so make sure you trust us :)

    Run in console
    ```bash
    make init_repo
    ````

* Manually:
    * [`clang`](https://clang.llvm.org/) compiler (tested with  version `10.0.0-4ubuntu1`). Example installation:
        ```bash
        sudo apt install clang
        ```
    * Linux display libraries. Example installation:
        ```bash
        sudo apt install libwayland-dev libxkbcommon-dev xorg-dev
        ```

    * CMake (tested version `3.29.3`). Example installation:
        ```bash
        sudo apt install cmake
        ```
    * (Optional) [Bear](https://github.com/rizsotto/Bear) tool (tested version `2.4.3`)
    
        Needed if you prefer to use `clangd` as development tool. `bear` simply dumps `clang` build commands into `compile_commands.json` file required by `clangd`.
        ```bash
        # to install
        sudo apt install bear
        # to run bear on this project
        make intellisense
        ```

Internal dependencies (you don't need to do anything to install):
* [ccache](https://ccache.dev/) `v4.8.3` for faster compilation
* [GLFW](https://github.com/glfw/glfw) `v3.4` for cross-platform window interaction
* [STB image](https://github.com/nothings/stb) `v2.30` for decoding images
* [GLAD](https://gen.glad.sh/) `v2.0.7` for fetching OpenGL function pointers
* [GLM](https://github.com/g-truc/glm) `v1.0.1` for math utilities
* [spdlog]() `v1.14.1` for logging
* [cr.h](https://github.com/fungos/cr) for simple hot reloading of dynamic libraries

## Build:
1. Clone the repository

    ```
    git clone --recurse-submodules https://github.com/laralex/RenderingExperiments.git
    ```

1. Run this to build everything (`-j16` for faster multi-threaded build)
    ```
    make -j16
    ```
    > NOTE: you can tweak build options in the `Makefile`. You can disable an option by commenting it out with `#`
    >
    > Changing of the options (except for `DEBUG`) would require a clean build (remove `build/debug` or `build/release` directory)
    > ```make
    > # For maximum logging, and debugger support (unoptimized code!)
    > DEBUG?=1
    >
    > # Start the application right after successful build
    > RUN_AFTER_BUILD?=1
    >
    > # Allows to recompile and update behavior of a running application. Builds the application code as a dynamic library.
    > USE_HOT_RELOADING?=1
    >
    > # For incremental compilation. Dumps GPU Make dependencies of all source files on header files
    > USE_DEP_FILES?=1
    >
    > # For faster re-compilation. Enables precompiled headers
    > USE_PCH?=1
    >
    > # For faster re-compilation. Enables caching of compiled files
    > USE_CCACHE?=1
    >
    > # For code completion (aka intellisense) in IDE. Uses Bear tool to generate `compile_commands.json` file
    > USE_CLANGD?=1
    >
    > # if ON,  link engine code dynamically into the application
    > # if OFF, link engine code statically into the application
    > USE_DYNLIB_ENGINE?=1
    >
    > # For debugging. Dumps more logs from compiler
    > USE_COMPILER_DUMP?=1
    > ```

4. If the application hasn't started yet, run it
    ```bash
    make run
    ```

5. If the application is running, and you want to recompile it without stopping the application, you can use
    ```bash
    make hot
    ```
    After compilation completes, the application will apply the newly compiled code automatically. It has limitations though, some code changes might crash the program, because of incompatible binary data (e.g. if you change the definitions of `class`/`struct` fields)

## Installing the application

Everything needed to run the application is stored in the installation folder (typically `./build/release/install`).

You can copy the whole folder somewhere else, and then to run the application you need:
```bash
cd <where_you_copied_install_directory>
LD_LIBRARY_PATH=. ./app
```
