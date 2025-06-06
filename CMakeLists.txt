cmake_minimum_required(VERSION 3.10)

include(cmake/randomize_pdb_name.cmake)

# TODO(lucas): For now, all executables and libraries built will go directly into the build directory.
# Once there is more profiling and testing, they can be spilt back into debug and release folders.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# TODO(lucas): Set C standard and disable C extensions
project(alchemy VERSION 0.1)

option(ALCHEMY_INCLUDE_EXAMPLES "Include example projects" OFF)
option(ALCHEMY_NO_HOT_RELOAD "Disable hot reloading through game DLL" OFF)
option(ALCHEMY_CONSOLE "Enable debug console" ON)

SET(STARTUP "example" CACHE STRING "Project in examples folder to run on startup")

set(ALCHEMY_SOURCE
    ${PROJECT_SOURCE_DIR}/lib/glad/src/glad.c
    ${PROJECT_SOURCE_DIR}/lib/stb_image/stb_image.c
    ${PROJECT_SOURCE_DIR}/lib/nuklear/nuklear.c
    ${PROJECT_SOURCE_DIR}/src/renderer/font.c
    ${PROJECT_SOURCE_DIR}/src/renderer/renderer.c
    ${PROJECT_SOURCE_DIR}/src/renderer/shader.c
    ${PROJECT_SOURCE_DIR}/src/renderer/sprite.c
    ${PROJECT_SOURCE_DIR}/src/renderer/texture.c
    ${PROJECT_SOURCE_DIR}/src/renderer/ui.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_file.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_input.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_intrin.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_memory.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_opengl.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_sound.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_state.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_window.c
    ${PROJECT_SOURCE_DIR}/src/util/log.c
    ${PROJECT_SOURCE_DIR}/src/util/time.c)

set(ALCHEMY_LIBS
    user32.lib
    gdi32.lib
    opengl32.lib
    xinput.lib
    xaudio2.lib
    cglm_headers)

# C4100 and C4189 are about unused parameters/variables and are not particularly useful
# C4116 is unnamed type definitions in parenthetical expressions. Disabled because of nuklear
# /Oi: Generate intrinsic functions
set(COMMON_COMPILER_FLAGS
    /Oi /WX /W4 /wd4100 /wd4189 /wd4201 /wd4005 /wd4116 /wd4090
    /D_CRT_SECURE_NO_WARNINGS)

add_library(alchemy ${ALCHEMY_SOURCE})
target_include_directories(alchemy PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(alchemy SYSTEM PUBLIC
                            ${PROJECT_SOURCE_DIR}/lib
                            ${PROJECT_SOURCE_DIR}/lib/glad/include
                            ${PROJECT_SOURCE_DIR}/lib/cglm
                            ${PROJECT_SOURCE_DIR}/lib/freetype/include)

target_link_libraries(alchemy PRIVATE ${ALCHEMY_LIBS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(alchemy PRIVATE ${PROJECT_SOURCE_DIR}/lib/freetype/debug/freetype.lib)
else()
    target_link_libraries(alchemy PRIVATE ${PROJECT_SOURCE_DIR}/lib/freetype/release/freetype.lib)
endif()

option(CGLM_STATIC "Static build" ON)
add_subdirectory(lib/cglm/ EXCLUDE_FROM_ALL)

target_compile_definitions(alchemy PRIVATE
                           ALCHEMY_INSTALL_PATH="${CMAKE_CURRENT_LIST_DIR}"
                           CMAKE_EXPORT_COMPILE_COMMANDS=1)
if(MSVC)
    target_compile_options(alchemy PRIVATE ${COMMON_COMPILER_FLAGS})
    target_compile_definitions(alchemy PUBLIC VC_EXTRALEAN WIN32_LEAN_AND_MEAN NOMINMAX)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # In debug mode, enable complete debug information (/Zi) and disable optimization (/Od)
        # Specify the multithreaded debug version of CRT (MTd) and perform all available run-time error checks (RTC1)
        set(DEBUG_COMPILER_FLAGS /MTd /Zi /Od /RTC1)
        target_compile_options(alchemy PRIVATE ${DEBUG_COMPILER_FLAGS})
        target_compile_definitions(alchemy PUBLIC ALCHEMY_DEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(RELEASE_COMPILER_FLAGS /MT /O2)
        target_compile_options(alchemy PRIVATE ${RELEASE_COMPILER_FLAGS})
    endif()
endif()

if(ALCHEMY_INCLUDE_EXAMPLES)
    add_subdirectory(examples/example)
    add_subdirectory(examples/snake)
endif()
if(ALCHEMY_NO_HOT_RELOAD)
    target_compile_definitions(alchemy PUBLIC ALCHEMY_NO_HOT_RELOAD)
endif()
if(ALCHEMY_CONSOLE)
    target_compile_definitions(alchemy PUBLIC ALCHEMY_CONSOLE)
endif()
