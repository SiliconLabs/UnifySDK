# Library
add_library(transport_service_wrapper
  src/zwave_transport_service_wrapper.c
)

target_include_directories(transport_service_wrapper
  PUBLIC include
  PRIVATE src
         transport_service
)

target_link_libraries(transport_service_wrapper
  PUBLIC zwave_controller
         zwave_tx
         zwave_rx
         zwave_s0
         zwave_s2
         zwave_multi_channel
         transport_service)

add_subdirectory(transport_service)
