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

SET(STARTUP "example" CACHE STRING "Project in examples folder to run on startup")

add_library(alchemy
               ${PROJECT_SOURCE_DIR}/lib/glad/src/glad.c
               ${PROJECT_SOURCE_DIR}/lib/stb_image/stb_image.c
               ${PROJECT_SOURCE_DIR}/lib/nuklear/nuklear.c
               ${PROJECT_SOURCE_DIR}/src/renderer/font.c
               ${PROJECT_SOURCE_DIR}/src/renderer/renderer.c
               ${PROJECT_SOURCE_DIR}/src/renderer/shader.c
               ${PROJECT_SOURCE_DIR}/src/renderer/sprite.c
               ${PROJECT_SOURCE_DIR}/src/renderer/texture.c
               ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_input.c
               ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_memory.c
               ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_sound.c
               ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_window.c
               ${PROJECT_SOURCE_DIR}/src/util/time.c
               ${PROJECT_SOURCE_DIR}/src/ui.c)

target_include_directories(alchemy PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_include_directories(alchemy SYSTEM PUBLIC
                            ${PROJECT_SOURCE_DIR}/lib
                            ${PROJECT_SOURCE_DIR}/lib/glad/include
                            ${PROJECT_SOURCE_DIR}/lib/cglm
                            ${PROJECT_SOURCE_DIR}/lib/freetype/include)

target_link_libraries(alchemy PRIVATE
                        user32.lib
                        gdi32.lib
                        opengl32.lib
                        xinput.lib
                        xaudio2.lib
                        cglm_headers
                        ${PROJECT_SOURCE_DIR}/lib/freetype/freetype.lib)

option(CGLM_STATIC "Static build" ON)
add_subdirectory(lib/cglm/ EXCLUDE_FROM_ALL)

set_target_properties(alchemy PROPERTIES 
                        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
                        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/res)

# TODO(lucas): Pull out common flags
if (MSVC)
    # C4100 and C4189 are about unused parameters/variables and are not particularly useful
    # C4116 is unnamed type definitions in parenthetical expressions. Disabled because of nuklear
    # /Oi: Generate intrinsic functions
    # LNK4099 is about linking without debug info. Disabled because of freetype
    target_compile_options(alchemy PRIVATE
                            /Oi /WX /W4 /wd4100 /wd4189 /wd4201 /wd4005 /wd4116
                            /D_CRT_SECURE_NO_WARNINGS)
    target_link_options(alchemy PRIVATE /subsystem:windows /entry:mainCRTStartup /ignore:4099)

    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        # In debug mode, enable complete debug information (/Zi) and disable optimization (/Od)
        target_compile_options(alchemy PRIVATE /Zi /Od /RTCcsu)
        target_compile_definitions(alchemy PRIVATE ALCHEMY_DEBUG)
    endif()
endif()

add_subdirectory(examples/example)
add_subdirectory(examples/snake)
