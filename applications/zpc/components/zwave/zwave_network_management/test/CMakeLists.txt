target_add_unittest(zwave_network_management
SOURCES
  zwave_network_management_test.c
DEPENDS
  uic_contiki_stub
  zwave_s2_mock
  zwave_controller_mock
  zwave_tx_mock
  zwave_api_mock
  zwave_s0_mock
)

## SmartStart learn mode tests
target_add_unittest(zwave_network_management
NAME      zwave_network_management_smartstart_learn_mode_test
SOURCES   zwave_network_management_smartstart_learn_mode_test.c
DEPENDS
  uic_contiki_stub
  zwave_api_mock
  zwave_s2_mock
  zwave_s0_mock
  zwave_network_management_helpers_mock
EXCLUDE
  zwave_network_management_helpers.c
)
target_include_directories(zwave_network_management_smartstart_learn_mode_test PRIVATE ../src)

## learn mode tests
target_add_unittest(zwave_network_management
NAME      zwave_network_management_learn_mode_test
SOURCES   zwave_network_management_learn_mode_test.c
DEPENDS
  uic_contiki_stub
  zwave_api_mock
  zwave_s2_mock
  zwave_s0_mock
  zwave_network_management_helpers_mock
EXCLUDE
  zwave_network_management_helpers.c
)
target_include_directories(zwave_network_management_learn_mode_test PRIVATE ../src)

# Test of the helper functions
target_add_unittest(zwave_network_management
NAME      zwave_network_management_helpers_test
SOURCES   zwave_network_management_helpers_test.c
DEPENDS
  zwave_api_mock
  zwave_controller_mock
)
target_include_directories(zwave_network_management_helpers_test PRIVATE ../src)

# Test of the callback functions
target_add_unittest(zwave_network_management
NAME      zwave_network_management_callbacks_test
SOURCES   zwave_network_management_callbacks_test.c
DEPENDS
  zwave_network_management_helpers_mock
EXCLUDE
  zwave_network_management_helpers.c
)
target_include_directories(zwave_network_management_callbacks_test PRIVATE ../src)


# Test of the logging functions
target_add_unittest(zwave_network_management
NAME      zwave_network_management_state_logging_test
SOURCES   zwave_network_management_state_logging_test.c
)
target_include_directories(zwave_network_management_state_logging_test PRIVATE ../src)

# Test of the Assign Return Route Queue
target_add_unittest(zwave_network_management
NAME      zwave_network_management_return_route_queue_test
SOURCES   zwave_network_management_return_route_queue_test.c
DEPENDS
  uic_contiki_stub
  zwave_api_mock
  zwave_s2_mock
  zwave_s0_mock
  zwave_network_management_helpers_mock
EXCLUDE
  zwave_network_management_helpers.c
)
target_include_directories(zwave_network_management_return_route_queue_test PRIVATE ../src)

## Set Default tests
target_add_unittest(zwave_network_management
NAME      zwave_network_management_set_default_test
SOURCES   zwave_network_management_set_default_test.c
DEPENDS
  uic_contiki_stub
  zwave_api_mock
  zwave_s2_mock
  zwave_s0_mock
  zwave_network_management_helpers_mock
  zwave_rx_mock
EXCLUDE
  zwave_network_management_helpers.c
)
target_include_directories(zwave_network_management_set_default_test PRIVATE ../src)

## Various state tests
target_add_unittest(zwave_network_management
NAME      zwave_network_management_various_test
SOURCES   zwave_network_management_various_test.c
DEPENDS
  uic_contiki_stub
  zwave_api_mock
  zwave_s2_mock
  zwave_s0_mock
  zwave_network_management_helpers_mock
  zwave_rx_mock
EXCLUDE
  zwave_network_management_helpers.c
)
target_include_directories(zwave_network_management_various_test PRIVATE ../src)

## Various state tests
target_add_unittest(zwave_network_management
NAME      zwave_network_management_neighbor_discovery_test
SOURCES   zwave_network_management_neighbor_discovery_test.c
DEPENDS
  uic_contiki_stub
  zwave_api_mock
  zwave_s2_mock
  zwave_s0_mock
  zwave_network_management_helpers_mock
  zwave_rx_mock
EXCLUDE
  zwave_network_management_helpers.c
)
target_include_directories(zwave_network_management_neighbor_discovery_test PRIVATE ../src)