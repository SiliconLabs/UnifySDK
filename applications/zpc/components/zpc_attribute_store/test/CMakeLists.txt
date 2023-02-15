cmake_path(SET path "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET path PARENT_PATH include_dir)
cmake_path(APPEND include_dir include)
if(NOT EXISTS ${include_dir}/zpc_attribute_store_network_helper.h)
  message(FATAL_ERROR "The file ${include_dir}/zpc_attribute_store_network_helper.h does not exist")
endif()

add_mock(zpc_attribute_store_network_helper_mock
         ${include_dir}/zpc_attribute_store_network_helper.h)
target_include_directories(
  zpc_attribute_store_network_helper_mock
  PUBLIC "$<TARGET_PROPERTY:unify,INTERFACE_INCLUDE_DIRECTORIES>"
         "$<TARGET_PROPERTY:zpc_utils,INTERFACE_INCLUDE_DIRECTORIES>")
target_link_libraries(zpc_attribute_store_network_helper_mock
                      PUBLIC ucl_definitions)

# Attribute store network helper Unit Test
target_add_unittest(
  zpc_attribute_store
  NAME
  zpc_attribute_store_network_helper_test
  SOURCES
  zpc_attribute_store_network_helper_test.c
  DEPENDS
  zwave_network_management_mock)

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

# Association helper test
target_add_unittest(
  zpc_attribute_store
  NAME
  zwave_association_toolbox_test
  SOURCES
  zwave_association_toolbox_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_network_management_mock)
