# Smartstart Manager DSK Parsing Tests
target_add_unittest(
  zigpc_smartstart
  NAME
  zigpc_smartstart_dsk_test
  SOURCES
  zigpc_smartstart_dsk_test.cpp
  DEPENDS
  zigpc_gateway_mock)

# Smartstart Manager Interface Tests
target_add_unittest(
  zigpc_smartstart
  NAME
  zigpc_smartstart_test
  SOURCES
  zigpc_smartstart_test.cpp
  DEPENDS
  zigpc_gateway_mock
  zigpc_net_mgmt_mock
  zigpc_common_mock
  uic_mqtt_mock
  uic_log
  uic_contiki_stub)
