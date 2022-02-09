# Notify Tests
target_add_unittest(
  zigpc_gateway
  NAME
  zigpc_gateway_notify_test
  SOURCES
  zigpc_gateway_notify_test.c
  DEPENDS
  zigbee_host_mock
  zigpc_common_mock)

# Public API Tests
target_add_unittest(
  zigpc_gateway
  SOURCES
  zigpc_gateway_test.c
  EXCLUDE
  zigpc_gateway_process.c
  DEPENDS
  zigbee_host_mock
  zigpc_datastore_mock
  uic_contiki_stub)

# Datastore Tests
target_add_unittest(
  zigpc_gateway
  NAME
  zigpc_gateway_datastore_test
  SOURCES
  zigpc_gateway_datastore_test.cpp
  DEPENDS
  zigbee_host_mock
  zigpc_datastore_mock
  uic_datastore_mock)

target_add_unittest(
  zigpc_gateway
  NAME
  zigpc_gateway_counters_test
  SOURCES
  zigpc_gateway_counters_test.cpp
  DEPENDS
  zigbee_host_mock)
