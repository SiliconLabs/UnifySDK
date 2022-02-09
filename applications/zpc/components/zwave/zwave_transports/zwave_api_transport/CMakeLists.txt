# Library
add_library(zwave_api_transport
  src/zwave_api_transport.c
)

target_include_directories(zwave_api_transport
  PUBLIC include
)

target_link_libraries(zwave_api_transport
  PRIVATE
    zwave_api
    zwave_tx_groups
    uic_contiki
    zwave_controller
)

target_add_mock(zwave_api_transport)

add_subdirectory(test)
