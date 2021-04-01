INF584-Project
==============

This is the final project for my university's course on Graphics Rendering.

Building
--------

Ensure you have a C++20 complient compiler, `gcc-10` works. Also, remember to install the development version of GLFW, `libglfw3-dev` on Ubuntu. Then type the following commands to build it as a cmake project:

    mkdir build && cd build
    cmake ..
    make

Running
-------

**Warning:** make sure you run the executable on the root folder of the repository (i.e. the parent directory to *resources*), otherwise the executable cannot find the shaders  required for it to work.

From the root folder, type:

    ./build/INF584Project

License
-------

This source code is under the MIT License. As such, all permissions apply to the code.
