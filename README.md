## C++/OpenGL rendering engine

Originated from famous [Learnopengl.com](https://learnopengl.com/) tutorial.

Now it's a playground for my simple graphical experiments - more concerned about rendering algorithms and scientific papers, less concerned about maximizing performance (though within realtime) 

Features:
- the engine code is isolated from application as a compiled library

## Build

Developed and tested on Ubuntu 20.04 (Wayland)

1. Clone the repository

2. Execute in terminal (only required once to install necessary packages and fetch submodules)
    ```
    make init_repo
    ```

3. Execute in terminal to build the engine and the application
    ```
    make
    ```

4. Run the application
    ```
    make run
    ```
