cmake_minimum_required(VERSION 3.10)

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
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_input.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_memory.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_opengl.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_sound.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_state.c
    ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_window.c
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
<<<<<<< HEAD
    /Oi /WX /W4 /wd4100 /wd4189 /wd4201 /wd4005 /wd4116
=======
    /Oi /WX /W4 /wd4100 /wd4189 /wd4201 /wd4005 /wd4116 /wd4090
>>>>>>> ebd83c9268a6a9fec3725ad1abd65f4521e57b33
    /D_CRT_SECURE_NO_WARNINGS)

# In debug mode, enable complete debug information (/Zi) and disable optimization (/Od)
# Specify the multithreaded debug version of CRT (MTd) and perform all available run-time error checks (RTC1)
set(DEBUG_COMPILER_FLAGS /MTd /Zi /Od /RTC1)

add_library(alchemy ${ALCHEMY_SOURCE})
target_include_directories(alchemy PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(alchemy SYSTEM PUBLIC
                            ${PROJECT_SOURCE_DIR}/lib
                            ${PROJECT_SOURCE_DIR}/lib/glad/include
                            ${PROJECT_SOURCE_DIR}/lib/cglm
                            ${PROJECT_SOURCE_DIR}/lib/freetype/include)

target_link_libraries(alchemy PRIVATE ${ALCHEMY_LIBS})

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    target_link_libraries(alchemy PRIVATE ${PROJECT_SOURCE_DIR}/lib/freetype/debug/freetype.lib)
else()
    target_link_libraries(alchemy PRIATE ${PROJECT_SOURCE_DIR}/lib/freetype/release/freetype.lib)
endif()

option(CGLM_STATIC "Static build" ON)
add_subdirectory(lib/cglm/ EXCLUDE_FROM_ALL)

target_compile_definitions(alchemy PRIVATE
                           ALCHEMY_INSTALL_PATH="${CMAKE_CURRENT_LIST_DIR}"
                           CMAKE_EXPORT_COMPILE_COMMANDS=1)
if(MSVC)
    target_compile_options(alchemy PRIVATE ${COMMON_COMPILER_FLAGS})

    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        target_compile_options(alchemy PRIVATE ${DEBUG_COMPILER_FLAGS})
        target_compile_definitions(alchemy PUBLIC ALCHEMY_DEBUG)
    endif()
endif()

macro(RANDOMIZE_PDB_NAME target)
    string(RANDOM random_string)
    set(pdbname "${target}_")
    string(APPEND pdbname ${random_string})
    set_target_properties(${target}
        PROPERTIES
        COMPILE_PDB_NAME ${pdbname}
        PDB_NAME ${pdbname})
endmacro()

if(ALCHEMY_INCLUDE_EXAMPLES)
    add_subdirectory(examples/example)
    add_subdirectory(examples/snake)
endif()
