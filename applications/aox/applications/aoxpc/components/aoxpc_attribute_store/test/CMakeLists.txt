
# Attribute store network helper Unit Test
target_add_unittest(aoxpc_attribute_store
  NAME aoxpc_attribute_store_network_helper_test
  SOURCES aoxpc_attribute_store_network_helper_test.c
)

# AoXPC Attribute store Unit Test
target_add_unittest(aoxpc_attribute_store
  NAME aoxpc_attribute_store_test
  SOURCES aoxpc_attribute_store_test.c
  DEPENDS aoxpc_unid_mock
)

# AoXPC Attribute Store dotdot Attributes unit test
target_add_unittest(aoxpc_attribute_store
  NAME aoxpc_attribute_store_dotdot_attributes_test
  SOURCES aoxpc_attribute_store_dotdot_attributes_test.c
)