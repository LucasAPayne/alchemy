# Alchemy Engine

## Requirements

- Windows 10
- CMake >= 3.10

## How to Build

Currently, this project only supports Windows. First, clone the repository with

    git clone --recursive https://github.com/LucasAPayne/alchemy.git

To build and run the project, simply navigate to the project directory and run

    build
    run

Note that the executable should be run from the `res` directory in order to load resources like textures, shaders, and fonts. If the project is run without the batch file, it should be run like the following:

    cd res
    start ../build/alchemy.exe

If running from a Visual Studio solution, make sure to set the working directory to `res` in Properties > Debugging > Working Directory.
