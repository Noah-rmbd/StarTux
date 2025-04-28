# StarTux is a game using OpenGl in C++

This is the code of the game, all you need is to build it using CMake. For VSCode, type "CMake: build" and insert the "CMakeLists.txt" file if required. Then "CMake: Compile" and you're done. 

# Installation

You'll need the latest version of CMake (to build our project), which can be downloaded from [here] (https://cmake.org/download/).

You'll need Freetype (https://github.com/freetype/freetype) and Assimp(https://github.com/assimp/assimp) 

# Setup

After cloning this repo, you also need to load the submodules GLEW, GLFW and GLM using the following command.

    git submodule update --init --recursive


## Run the Application

To launch the window, simply run:

    cmake .
    make
    ./opengl_program

## Controls

There are two modes : dev mode (c button) and play mode (x button)
When running the game is in dev mode. Use the buttons above to switch between the modes.

In dev mode :
    Z = Move the camera forward
    Q = Mode the camera to the left
    S = Move the camera backward
    D = Move the camera to the right
    R = Go up
    F = Go down
    Mouse = Controls the direction of the camera

In game mode :
    U = Down
    J = Up
    H = Left
    K = Right
    O = Left Roll
    P = Right Roll
