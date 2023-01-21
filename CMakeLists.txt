cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_COMPILER )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project(alchemy)

add_executable(${PROJECT_NAME} WIN32
               ${PROJECT_SOURCE_DIR}/src/win32_alchemy.cpp
               ${PROJECT_SOURCE_DIR}/src/example.cpp
               ${PROJECT_SOURCE_DIR}/lib/glad/src/glad.c
               ${PROJECT_SOURCE_DIR}/lib/stb_image/stb_image.c
               ${PROJECT_SOURCE_DIR}/src/renderer/font.cpp
               ${PROJECT_SOURCE_DIR}/src/renderer/shader.cpp
               ${PROJECT_SOURCE_DIR}/src/renderer/sprite.cpp
               ${PROJECT_SOURCE_DIR}/src/renderer/texture.cpp
               ${PROJECT_SOURCE_DIR}/src/platform/windows/win32_input.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
            ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_SOURCE_DIR}/lib
            ${PROJECT_SOURCE_DIR}/lib/glad/include
            ${PROJECT_SOURCE_DIR}/lib/glm
            ${PROJECT_SOURCE_DIR}/lib/freetype/include)

target_link_libraries(${PROJECT_NAME} PRIVATE
                      user32.lib
                      gdi32.lib
                      opengl32.lib
                      ${PROJECT_SOURCE_DIR}/lib/freetype/freetype.lib)

set_target_properties(${PROJECT_NAME}
                      PROPERTIES 
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})

if (MSVC)
    # C4100 and C4189 are about unused parameters/variables and are not particularly useful
    # /Oi: Generate intrinsic functions
    target_compile_options(${PROJECT_NAME} PRIVATE /Oi /WX /W4 /wd4100 /wd4189 /wd4201)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    # In debug mode, enable complete debug information (/Zi) and disable optimization (/Od)
    target_compile_options(${PROJECT_NAME} PRIVATE /Zi /Od)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ALCHEMY_DEBUG)
endif()
