add_executable(snake main.c)
target_link_libraries(snake PRIVATE alchemy cglm_headers)
set_target_properties(snake PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/res)

if(MSVC)
    target_compile_options(snake PRIVATE ${COMMON_COMPILER_FLAGS})
    target_link_options(snake PRIVATE /subsystem:windows /entry:mainCRTStartup)

    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        target_compile_options(snake PRIVATE ${DEBUG_COMPILER_FLAGS})
    endif()
endif()
