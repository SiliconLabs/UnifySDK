# Library
add_library(multicast_follow_ups_transport
  src/multicast_follow_ups_transport.c
)

target_include_directories(multicast_follow_ups_transport PUBLIC include)

target_link_libraries(multicast_follow_ups_transport
  PRIVATE
    zwave_tx_groups
    uic_contiki
    zwave_controller
)

target_add_mock(multicast_follow_ups_transport)

add_subdirectory(test)
