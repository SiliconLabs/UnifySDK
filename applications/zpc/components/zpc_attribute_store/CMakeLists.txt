# Association toolbox library
add_library(
  zwave_association_toolbox
  src/zwave_association_toolbox.cpp
)
target_include_directories(
  zwave_association_toolbox
  PUBLIC include include/command_class_types
  PRIVATE src)

target_link_libraries(
  zwave_association_toolbox
  PUBLIC zwave_definitions ucl_definitions
  PRIVATE uic_attribute_store zwave_controller dotdot_mapper_binding_cluster_helper
)

# ZPC attribute store Library
add_library(
  zpc_attribute_store
  src/zpc_attribute_store.c
  src/zpc_attribute_store_type_registration.cpp
  src/zpc_attribute_store_register_default_attribute_type_data.cpp
  src/zpc_attribute_store_network_helper.c
  src/zwave_utils.c
)

target_include_directories(
  zpc_attribute_store
  PUBLIC include include/command_class_types
  PRIVATE src)

target_link_libraries(
  zpc_attribute_store
  PUBLIC ucl_definitions uic_attribute_store zpc_utils zwave_association_toolbox
  PRIVATE zwave_handlers
  uic_dotdot_attribute_store
  zwave_controller
)
install(TARGETS zpc_attribute_store LIBRARY DESTINATION lib)

# Mocks We do not use target_add_mock here, because we do not want to mock
# attribute_store_defined_attribute_types.h
add_mock(zpc_attribute_store_mock
  include/zpc_attribute_store.h
  include/zwave_utils.h
  include/zpc_attribute_store_network_helper.h)

target_include_directories(
  zpc_attribute_store_mock
  PUBLIC "$<TARGET_PROPERTY:uic_attribute_store,INTERFACE_INCLUDE_DIRECTORIES>"
  "$<TARGET_PROPERTY:zpc_utils,INTERFACE_INCLUDE_DIRECTORIES>"
  include/command_class_types)
target_link_libraries(zpc_attribute_store_mock PUBLIC ucl_definitions)

add_subdirectory(test)
add_subdirectory(test_helper)
