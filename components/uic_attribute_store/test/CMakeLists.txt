# Attribute store Unit Test 
target_add_unittest(uic_attribute_store SOURCES attribute_store_test.c )

# Attribute store value read helper Unit Test 
target_add_unittest(uic_attribute_store NAME uic_attribute_store_helper_test SOURCES attribute_store_helper_test.c)

# Attribute store benchmark test (not really a unit test)
target_add_unittest(uic_attribute_store DISABLED NAME uic_attribute_store_benchmark_test SOURCES attribute_store_benchmark_test.c)

target_add_unittest(uic_attribute_store NAME uic_attribute_store_cpp_wrap_test SOURCES attribute_store_cpp_wrap_test.cpp)
