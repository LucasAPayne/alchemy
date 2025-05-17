# Alchemy Engine

## Requirements

- Windows 10
- CMake >= 3.10

## How to Build

Currently, this project only supports Windows. First, clone the repository with

```bat
git clone --recursive https://github.com/LucasAPayne/alchemy.git
```

To build all examples and run one, simply run

```bat
build
run example_name
```

where `example_name` is the name of the example within the `examples` folder you wish to run. For example, to run the snake example, the command would be `run snake`.

Note that the executable should be run from the `res` directory in order to load resources like textures, shaders, and fonts. If an example, like snake, is run without the batch file, it should be run like the following:

```bat
cd res
start ../build/snake.exe
```

## Embedding the Window Icon into the Executable

A window icon can be set any time after window creation from a file with `window_icon_load_from_file()` and `window_icon_set_from_memory()`. However, the icon can also be embedded directly into the executable using a `.rc` file. To do so, first create a file named something like `resource.h`, which will contain definitions for each resource. It could look something like this:

```c
// resource.h
#define IDI_APP_ICON 101
```

Next, create the `resource.rc` file, which links the definitions to files:

```c
// resource.rc
#include "resource.h"

IDI_APP_ICON ICON "icons/icon.ico"
```

Finally, load the icon *before* creating the window:

```c
// main.c
#include <alchemy/window.h>
#include "resource.h"

int main()
{
    window_icon_set_from_resource(IDI_APP_ICON);
    Window* window = window_create("Example", 1920, 1080);
    //...
}

```

In your `CMakeLists.txt`, copy the icon file to the build directory, and include the `.rc` along with any other source files:

```cmake
configure_file(${PROJECT_SOURCE_DIR}/icon.ico ${CMAKE_BINARY_DIR}/icon.ico COPYONLY)
add_executable(${PROJECT_NAME} main.c resource.rc)
```
