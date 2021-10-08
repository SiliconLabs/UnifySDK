# Smartstart Manager Interface Tests
target_add_unittest(
  zigpc_smartstart
  SOURCES
  zigpc_smartstart_test.cpp
  DEPENDS
  zigpc_net_mgmt_mock
  zigpc_common_mock
  uic_mqtt_mock
  uic_log
  uic_contiki_stub)
