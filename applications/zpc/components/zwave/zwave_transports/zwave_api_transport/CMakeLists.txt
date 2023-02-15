# Library
add_library(zwave_api_transport
  src/zwave_api_transport.c
)

target_include_directories(zwave_api_transport
  PUBLIC include
)

target_link_libraries(zwave_api_transport
  PUBLIC zwave_definitions
  PRIVATE
    zwave_tx_groups
    unify
    zwave_controller
)

if(BUILD_TESTING)
  target_add_mock(zwave_api_transport)

  add_subdirectory(test)
endif()
