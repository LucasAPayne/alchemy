cmake_minimum_required(VERSION 3.10)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project(alchemy VERSION 0.1)

add_executable(${PROJECT_NAME} WIN32
               ${PROJECT_SOURCE_DIR}/example/example.c
               ${PROJECT_SOURCE_DIR}/example/ui_overview.c
               ${PROJECT_SOURCE_DIR}/example/win32_alchemy.c
               ${PROJECT_SOURCE_DIR}/lib/glad/src/glad.c
               ${PROJECT_SOURCE_DIR}/lib/stb_image/stb_image.c
               ${PROJECT_SOURCE_DIR}/lib/nuklear/nuklear.c
               ${PROJECT_SOURCE_DIR}/src/renderer/font.c
               ${PROJECT_SOURCE_DIR}/src/renderer/renderer.c
               ${PROJECT_SOURCE_DIR}/src/renderer/shader.c
               ${PROJECT_SOURCE_DIR}/src/renderer/sprite.c
               ${PROJECT_SOURCE_DIR}/src/renderer/texture.c
               ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_input.c
               ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_sound.c
               ${PROJECT_SOURCE_DIR}/src/util/time.c
               ${PROJECT_SOURCE_DIR}/src/ui.c)

target_include_directories(${PROJECT_NAME} PUBLIC
            ${PROJECT_SOURCE_DIR}/include)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
            ${PROJECT_SOURCE_DIR}/lib
            ${PROJECT_SOURCE_DIR}/lib/glad/include
            ${PROJECT_SOURCE_DIR}/lib/cglm
            ${PROJECT_SOURCE_DIR}/lib/freetype/include)

target_link_libraries(${PROJECT_NAME} PRIVATE
                      user32.lib
                      gdi32.lib
                      opengl32.lib
                      xinput.lib
                      xaudio2.lib
                      cglm_headers
                      ${PROJECT_SOURCE_DIR}/lib/freetype/freetype.lib)

option(CGLM_STATIC "Static build" ON)
add_subdirectory(lib/cglm/ EXCLUDE_FROM_ALL)

set_target_properties(${PROJECT_NAME} PROPERTIES 
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
                      VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/res)

if (MSVC)
    # C4100 and C4189 are about unused parameters/variables and are not particularly useful
    # C4116 is unnamed type definitions in parenthetical expressions. Disabled because of nuklear
    # /Oi: Generate intrinsic functions
    # LNK4099 is about linking without debug info. Disabled because of freetype
    target_compile_options(${PROJECT_NAME} PRIVATE /Oi /WX /W4 /wd4100 /wd4189 /wd4201 /wd4005 /wd4116)
    target_link_options(${PROJECT_NAME} PRIVATE /ignore:4099)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    # In debug mode, enable complete debug information (/Zi) and disable optimization (/Od)
    target_compile_options(${PROJECT_NAME} PRIVATE /Zi /Od)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ALCHEMY_DEBUG)
endif()
