# Alchemy Engine

## Requirements

- Windows 10
- CMake >= 3.10

## How to Build

Currently, this project only supports Windows. First, clone the repository with

    git clone --recursive https://github.com/LucasAPayne/alchemy.git

To build all examples and run one, simply run

    build
    run example_name

where `example_name` is the name of the example within the `examples` folder you wish to run. For example, to run the snake example, the command would be `run snake`.

Note that the executable should be run from the `res` directory in order to load resources like textures, shaders, and fonts. If an example, like snake, is run without the batch file, it should be run like the following:

    cd res
    start ../build/snake.exe
