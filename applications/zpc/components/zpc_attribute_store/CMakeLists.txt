# Library
add_library(
  zpc_attribute_store
  src/zpc_attribute_store.c src/zpc_attribute_store_attr_type_name_helper.cpp
  src/zpc_attribute_store_network_helper.c src/zwave_utils.c)

target_include_directories(
  zpc_attribute_store
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  zpc_attribute_store
  PUBLIC zwave_definitions ucl_definitions uic_attribute_store
  INTERFACE zpc_utils
  PRIVATE zwave_network_management zwave_handlers dotdot_attributes zpc_utils)
install(TARGETS zpc_attribute_store LIBRARY DESTINATION lib)

# Mocks We do not use target_add_mock here, because we do not want to mock
# attribute_store_defined_attribute_types.h
add_mock(zpc_attribute_store_mock include/zpc_attribute_store.h
         include/zwave_utils.h include/zpc_attribute_store_network_helper.h)
target_include_directories(
  zpc_attribute_store_mock
  PUBLIC "$<TARGET_PROPERTY:uic_attribute_store,INTERFACE_INCLUDE_DIRECTORIES>"
         "$<TARGET_PROPERTY:zpc_utils,INTERFACE_INCLUDE_DIRECTORIES>")
add_subdirectory(test)
add_subdirectory(test_helper)
