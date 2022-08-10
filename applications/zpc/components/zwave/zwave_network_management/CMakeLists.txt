# Library
add_library(
  zwave_network_management
  src/nm_state_machine.c
  src/state_logging.c
  src/zwave_network_management.c
  src/zwave_network_management_fixt.c
  src/zwave_network_management_process.c
  src/zwave_network_management_helpers.c
  src/zwave_network_management_callbacks.c
  src/zwave_network_management_return_route_queue.c)

target_link_libraries(zwave_network_management
                      PUBLIC zwave_controller uic_contiki
                      PRIVATE zwave_s0 zwave_s2)
target_include_directories(zwave_network_management PUBLIC include)
install(TARGETS zwave_network_management LIBRARY DESTINATION lib)

# Unit tests
add_subdirectory(test)

## Mocks
target_add_mock(zwave_network_management)

# Helper mocks
add_mock(zwave_network_management_helpers_mock src/zwave_network_management_helpers.h)
target_link_libraries(zwave_network_management_helpers_mock
                      PUBLIC zwave_controller)
