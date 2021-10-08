add_mock(uic_attribute_resolver_rule_test_mock attribute_resolver_rule_test.h)
target_interface_libraries(uic_attribute_resolver_rule_test_mock uic_contiki
                           uic_attribute_store)

# Resolver test
target_add_unittest(
  uic_attribute_resolver
  SOURCES
  attribute_resolver_test.c
  attribute_resolver_rule_mock.cpp
  DEPENDS
  uic_contiki_stub
  uic_attribute_resolver_rule_mock
  EXCLUDE
  src/attribute_resolver_rule.cpp)

# Resolver test number 2 (common test for rule + resolver parts)
target_add_unittest(
  uic_attribute_resolver
  NAME
  attribute_resolver_common_test
  SOURCES
  attribute_resolver_common_test.cpp
  DEPENDS
  uic_contiki_stub
  uic_attribute_store
  uic_datastore
  uic_attribute_resolver_mock)

# Resolver Benchmark test
target_add_unittest(
  uic_attribute_resolver
  DISABLED
  NAME
  attribute_resolver_benchmark_test
  SOURCES
  attribute_resolver_benchmark_test.c
  attribute_resolver_rule_mock.cpp
  DEPENDS
  uic_attribute_resolver_rule_mock
  uic_log
  uic_contiki_stub
  EXCLUDE
  src/attribute_resolver_rule.cpp)
