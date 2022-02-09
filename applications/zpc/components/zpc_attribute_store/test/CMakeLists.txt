add_mock(zpc_attribute_store_network_helper_mock  ../include/zpc_attribute_store_network_helper.h)
target_include_directories(
  zpc_attribute_store_network_helper_mock
  PUBLIC "$<TARGET_PROPERTY:uic_attribute_store,INTERFACE_INCLUDE_DIRECTORIES>"
         "$<TARGET_PROPERTY:zpc_utils,INTERFACE_INCLUDE_DIRECTORIES>")

# Attribute store network helper Unit Test

target_add_unittest(
  zpc_attribute_store NAME zpc_attribute_store_network_helper_test SOURCES
  zpc_attribute_store_network_helper_test.c)

target_add_unittest(
  zpc_attribute_store
  NAME
  zpc_attribute_store_test
  SOURCES
  zpc_attribute_store_test.c
  DEPENDS
  zwave_network_management_mock)

target_add_unittest(
  zpc_attribute_store
  NAME
  zwave_utils_test
  SOURCES
  zwave_utils_test.c
  DEPENDS
  uic_attribute_store_mock
  zwave_controller_mock
  zpc_attribute_store_network_helper_mock
  EXCLUDE
  zpc_attribute_store_network_helper.c)
