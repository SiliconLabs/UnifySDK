# Mocks
add_mock(zigpc_gateway_process_send_mock ../src/zigpc_gateway_process_send.h)

# Notify Tests
target_add_unittest(
  zigpc_gateway
  NAME
  zigpc_gateway_notify_test
  SOURCES
  zigpc_gateway_notify_test.c
  DEPENDS
  z3gateway_mock
  zigpc_common_mock)

# Dispatch Tests
target_add_unittest(
  zigpc_gateway
  NAME
  zigpc_gateway_dispatch_test
  SOURCES
  zigpc_gateway_dispatch_test.c
  DEPENDS
  z3gateway_mock
  zigpc_common_mock)

# Public API Tests
target_add_unittest(
  zigpc_gateway
  SOURCES
  zigpc_gateway_test.c
  DEPENDS
  zigpc_gateway_process_send_mock
  z3gateway_mock
  zigpc_common_mock
  zigpc_node_mgmt_mock)

# Datastore Tests
target_add_unittest(
  zigpc_gateway
  NAME
  zigpc_gateway_datastore_test
  SOURCES
  zigpc_gateway_datastore_test.cpp
  DEPENDS
  z3gateway_mock
  uic_datastore_mock
  zigpc_common_mock)
