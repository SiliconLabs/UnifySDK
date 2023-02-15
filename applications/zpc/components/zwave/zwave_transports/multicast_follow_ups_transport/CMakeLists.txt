# Library
add_library(multicast_follow_ups_transport OBJECT
  src/multicast_follow_ups_transport.c
)

target_include_directories(multicast_follow_ups_transport PUBLIC include)

target_link_libraries(multicast_follow_ups_transport
  PRIVATE
    zwave_tx_groups
    unify
    zwave_controller
)

if(BUILD_TESTING)
  target_add_mock(multicast_follow_ups_transport)

  add_subdirectory(test)
endif()
