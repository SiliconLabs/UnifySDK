# ZPC attribute store Library
add_library(
  zpc_attribute_store
  src/zpc_attribute_store.c
  src/zpc_attribute_store_network_helper.c
  src/zpc_attribute_store_register_default_attribute_type_data.cpp
  src/zpc_attribute_store_type_registration.cpp
  src/zwave_association_toolbox.cpp
  src/zwave_utils.c)

target_include_directories(
  zpc_attribute_store
  PUBLIC include include/command_class_types
  PRIVATE src)

target_link_libraries(
  zpc_attribute_store
  PUBLIC ucl_definitions unify zpc_utils zwave_controller
  PRIVATE zwave_handlers dotdot_mapper_binding_cluster_helper)
install(TARGETS zpc_attribute_store LIBRARY DESTINATION lib)

if(BUILD_TESTING)
  # Mocks We do not use target_add_mock here, because we do not want to mock
  # attribute_store_defined_attribute_types.h
  add_mock(
    zpc_attribute_store_mock
    ${CMAKE_CURRENT_LIST_DIR}/include/zpc_attribute_store.h
    ${CMAKE_CURRENT_LIST_DIR}/include/zwave_utils.h
    ${CMAKE_CURRENT_LIST_DIR}/include/zpc_attribute_store_network_helper.h)

  target_include_directories(
    zpc_attribute_store_mock
    PUBLIC "$<TARGET_PROPERTY:unify,INTERFACE_INCLUDE_DIRECTORIES>"
           "$<TARGET_PROPERTY:zpc_utils,INTERFACE_INCLUDE_DIRECTORIES>"
           "$<TARGET_PROPERTY:zwave_controller,INTERFACE_INCLUDE_DIRECTORIES>"
           ${CMAKE_CURRENT_LIST_DIR}/include/command_class_types)
  target_link_libraries(zpc_attribute_store_mock PUBLIC ucl_definitions)

  add_subdirectory(test)
  add_subdirectory(test_helper)
endif()
