set(TEST_INCLUDE_HEADERS ../src ../include)

# Mocks
add_mock(zigpc_common_zigbee_mock ../include/zigpc_common_zigbee.h)
add_mock(zigpc_common_observable_test_mock mocks/zigpc_test_observer.h)

# Hash Map Tests
target_add_unittest(
  zigpc_common
  NAME
  zigpc_common_hash_map_test
  SOURCES
  hash_map_test.c
  DEPENDS
  zigpc_common_zigbee_mock)

# Callback Map Tests
target_add_unittest(
  zigpc_common
  NAME
  zigpc_common_callback_map_test
  SOURCES
  callback_map_test.c
  DEPENDS
  zigpc_common_zigbee_mock)

# Zigbee Helper Tests
target_add_unittest(zigpc_common NAME zigpc_common_zigbee_test SOURCES
                    zigpc_common_zigbee_test.c)

# Common UCL Tests
target_add_unittest(zigpc_common NAME zigpc_common_ucl_test SOURCES
                    zigpc_common_ucl_test.c)

# Observable Tests
target_add_unittest(
  zigpc_common
  NAME
  zigpc_common_observable_test
  SOURCES
  zigpc_common_observable_test.c
  DEPENDS
  zigpc_common_observable_test_mock)
