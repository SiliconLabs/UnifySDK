
target_add_unittest(zwave_smartstart_management
SOURCES
  zwave_smartstart_management_test.cpp
  smartstart_management_mock.cpp
DEPENDS
  zwave_controller_mock
  zpc_config_mock
  uic_contiki_stub
  zwave_network_management_mock
  uic_smartstart_management
  zwave_network_management
  zwave_smartstart_management
EXCLUDE
  smartstart_management.cpp
)