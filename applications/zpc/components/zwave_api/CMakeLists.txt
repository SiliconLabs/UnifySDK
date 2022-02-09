set(ZWAVE_API_SRC_FILES
    "src/zwapi_connection.c"
    "src/zwapi_init.c"
    "src/zwapi_protocol_basis.c"
    "src/zwapi_protocol_controller.c"
    "src/zwapi_protocol_mem.c"
    "src/zwapi_protocol_nvm.c"
    "src/zwapi_protocol_rx_dispatch.c"
    "src/zwapi_protocol_transport.c"
    "src/zwapi_session.c"
    "src/zwapi_utils.c"
    "platform/${COMPATIBLE_PLATFORM}/zwapi_serial.c"
    "platform/${COMPATIBLE_PLATFORM}/zwapi_timestamp.c")

include_directories(
  "src" # src one is needed for arch/linux/zwapi_timer.c to find its private
        # header
)

add_library(zwave_api ${ZWAVE_API_SRC_FILES})
target_link_libraries(zwave_api zpc_utils uic_log)
target_include_directories(
  zwave_api
  PRIVATE src platform/${COMPATIBLE_PLATFORM}
  PUBLIC include)
install(TARGETS zwave_api LIBRARY DESTINATION lib)

target_add_mock(zwave_api)

# Mock within the component
add_mock(zwapi_internal_mock src/zwapi_connection.h src/zwapi_session.h)
target_interface_libraries(zwapi_internal_mock zpc_utils zwave_api)

add_library(
  zwapi_protocol_mem_mock
  ${ZPC_SOURCE_DIR}/components/zwave_api/mocks/zwapi_protocol_mem_mock.c)
target_interface_libraries(zwapi_protocol_mem_mock zwave_api)
add_subdirectory(test)
