# Add all source files containing game logic to the DLL
# Add only the main source file to the executable
add_library(example_dll SHARED ui_overview.c example.c)
add_executable(example main.c)

# cglm must be linked to use math functions
target_link_libraries(example_dll PRIVATE alchemy cglm_headers)
target_link_libraries(example PRIVATE alchemy cglm_headers)

# To make resource filepaths convenient, the Visual Studio working directory can be set to res.
# If this is used, the executable should also be called from the res direcotry outisde VS.
# These steps are not necessary, but it makes working with relative paths easier
set_target_properties(example PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/res)

if(MSVC)
    target_compile_options(example PRIVATE ${COMMON_COMPILER_FLAGS})
    target_compile_options(example_dll PRIVATE ${COMMON_COMPILER_FLAGS})

    # In order to use the main() function rather than WinMain() on Windows, use
    # /subsystem:windows and /entry:mainCRTStartup
    target_link_options(example PRIVATE /subsystem:windows /entry:mainCRTStartup)

    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        target_compile_options(example PRIVATE ${DEBUG_COMPILER_FLAGS})
        target_compile_options(example_dll PRIVATE ${DEBUG_COMPILER_FLAGS})
    endif()
endif()

# When hot reloading is performed during debugging, the PDB file can become locked.
# Randomizing its name prevents this issue.
RANDOMIZE_PDB_NAME(example_dll)
