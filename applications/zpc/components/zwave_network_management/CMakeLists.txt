# Library
add_library(
  zwave_network_management
  src/nm_state_machine.c
  src/state_logging.c
  src/zwave_network_management.c
  src/zwave_network_management_fixt.c
  src/zwave_network_management_process.c
)

target_link_libraries(
  zwave_network_management
  zwave_controller
  zwave_security_validation
  zwave_s2
  zwave_api
  uic_contiki
  zwave_s0
)
target_include_directories(zwave_network_management PUBLIC include)

add_subdirectory(test)

target_add_mock(zwave_network_management)

