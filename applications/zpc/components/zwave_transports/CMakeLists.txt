# Library
add_library(zwave_transports
  src/zwave_transports_fixt.c
  src/zwave_multi_channel_transport.c
  src/zwave_s0_transport.c
  src/zwave_transport_service_wrapper.c
)

target_include_directories(zwave_transports
  PUBLIC include
  PRIVATE src
         ${ZPC_SOURCE_DIR}/components/s2/libs/zw-libs2/include
         transport_service
)
target_compile_definitions(zwave_transports PRIVATE ZIPGW ZW_CONTROLLER)

target_link_libraries(zwave_transports
  PUBLIC zwave_controller
         zwave_tx
         zwave_rx
         zwave_security_validation
         zwave_s0
         transport_service)

add_subdirectory(test)
add_subdirectory(transport_service)
target_add_mock(zwave_transports)
