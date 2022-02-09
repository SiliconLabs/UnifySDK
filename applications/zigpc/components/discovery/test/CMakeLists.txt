add_mock(test_callbacks_mock mocks/zigpc_discovery_test_callbacks.h)
target_interface_libraries(test_callbacks_mock zigpc_discovery)

target_add_unittest(
  zigpc_discovery
  NAME
  zigpc_discovery_test
  SOURCES
  zigpc_discovery_test.cpp
  DEPENDS
  test_callbacks_mock
  zigpc_datastore_mock
  zigpc_gateway_mock
  uic_contiki_stub)

target_add_unittest(
  zigpc_discovery
  NAME
  zigpc_discovery_callbacks_test
  SOURCES
  zigpc_discovery_callbacks_test.cpp
  DEPENDS
  test_callbacks_mock
  zigpc_datastore_mock
  zigpc_gateway_mock
  uic_contiki_stub)

target_add_unittest(
  zigpc_discovery
  NAME
  zigpc_discovery_process_test
  SOURCES
  zigpc_discovery_process_test.cpp
  DEPENDS
  zigpc_datastore_mock
  zigpc_gateway_mock
  uic_contiki_stub)

target_add_unittest(
  zigpc_discovery
  NAME
  zigpc_discovery_timers_test
  SOURCES
  zigpc_discovery_timers_test.cpp
  DEPENDS
  test_callbacks_mock
  zigpc_datastore_mock
  zigpc_gateway_mock
  uic_contiki_stub)
