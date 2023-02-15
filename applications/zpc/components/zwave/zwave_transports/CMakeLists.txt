# Library
add_library(zwave_transports src/zwave_transports_fixt.c)

target_include_directories(
  zwave_transports
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  zwave_transports
  PUBLIC zwave_controller
         zwave_tx
         zwave_rx
         zwave_s0
         zwave_s2
         zwave_api_transport
         zwave_definitions
         multicast_follow_ups_transport
         zwave_multi_channel
         transport_service_wrapper
         zwave_tx_scheme_selector)
install(TARGETS zwave_transports LIBRARY DESTINATION lib)

add_subdirectory(s0)
add_subdirectory(s2)
add_subdirectory(multi_channel)
add_subdirectory(multicast_follow_ups_transport)
add_subdirectory(zwave_api_transport)
add_subdirectory(transport_service_wrapper)

if(BUILD_TESTING)
  # Unit tests
  add_subdirectory(test)
endif()
