add_library(example_dll SHARED
            # ui_overview.c
            example.c)
add_executable(example main.c)

target_include_directories(example_dll PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(example_dll PRIVATE alchemy cglm_headers)
target_link_libraries(example PRIVATE alchemy cglm_headers)

set_target_properties(example PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/res)

if (MSVC)
    # C4100 and C4189 are about unused parameters/variables and are not particularly useful
    # C4116 is unnamed type definitions in parenthetical expressions. Disabled because of nuklear
    # /Oi: Generate intrinsic functions
    # LNK4099 is about linking without debug info. Disabled because of freetype
    target_compile_options(example PRIVATE
                            /Oi /WX /W4 /wd4100 /wd4189 /wd4201 /wd4005 /wd4116
                            /D_CRT_SECURE_NO_WARNINGS)
    target_link_options(example PRIVATE /subsystem:windows /entry:mainCRTStartup /ignore:4099)

    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        # In debug mode, enable complete debug information (/Zi) and disable optimization (/Od)
        target_compile_options(example PRIVATE /Zi /Od /RTCcsu)
        # target_compile_definitions(example PRIVATE ALCHEMY_DEBUG)
    endif()
endif()
