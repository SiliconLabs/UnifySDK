# Source files
set(ZWAVE_API_DEMO_SRC_FILES
    src/zwave_api_demo.c src/zwave_api_demo_callbacks.c
    src/zwave_api_demo_commands.c
    platform/${COMPATIBLE_PLATFORM}/command_line_input.c)

add_executable(zwave_api_demo ${ZWAVE_API_DEMO_SRC_FILES})

target_link_libraries(zwave_api_demo zwave_api zpc_utils)
