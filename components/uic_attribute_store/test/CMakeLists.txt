# Attribute store Unit Test
target_add_unittest(uic_attribute_store SOURCES attribute_store_test.c )

# Attribute store value read helper Unit Test
target_add_unittest(uic_attribute_store NAME uic_attribute_store_helper_test SOURCES attribute_store_helper_test.c)

# Attribute store configuration test
target_add_unittest(
  uic_attribute_store
  NAME uic_attribute_store_configuration_test
  SOURCES attribute_store_configuration_test.c
)

# Attribute store save to datastore test
target_add_unittest(
  uic_attribute_store
  NAME attribute_store_save_to_datastore_test
  SOURCES attribute_store_save_to_datastore_test.c
)
target_include_directories(attribute_store_save_to_datastore_test PRIVATE ../src)

# Attribute store validation test
target_add_unittest(
  uic_attribute_store
  NAME uic_attribute_store_validation_test
  SOURCES attribute_store_validation_test.c
)

# Attribute store benchmark test (not really a unit test)
target_add_unittest(uic_attribute_store DISABLED NAME uic_attribute_store_benchmark_test SOURCES attribute_store_benchmark_test.c)

target_add_unittest(uic_attribute_store NAME uic_attribute_store_cpp_wrap_test SOURCES attribute_store_cpp_wrap_test.cpp)

# Attribute Store type registration test
target_add_unittest(uic_attribute_store
  NAME uic_attribute_store_type_registration_test
  SOURCES attribute_store_type_registration_test.c)
