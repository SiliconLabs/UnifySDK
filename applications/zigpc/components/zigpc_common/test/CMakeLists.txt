set(TEST_INCLUDE_HEADERS ../src ../include)

# Mocks
add_mock(zigpc_common_observable_test_mock mocks/zigpc_test_observer.h)

# Zigbee Helper Tests
target_add_unittest(zigpc_common NAME zigpc_common_zigbee_test SOURCES
                    zigpc_common_zigbee_test.c)

# Observable Tests
target_add_unittest(
  zigpc_common
  NAME
  zigpc_common_observable_test
  SOURCES
  zigpc_common_observable_test.c
  DEPENDS
  zigpc_common_observable_test_mock)
