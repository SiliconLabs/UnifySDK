# ZAP is required here
find_program(ZAP_EXECUTABLE "zap" REQUIRED)
run_zap(zap/gen-templates.json)

# Library
add_library(aoxpc_attribute_store
  src/aoxpc_attribute_store.c
  src/aoxpc_attribute_store_network_helper.c
  ${CMAKE_CURRENT_BINARY_DIR}/src/aoxpc_attribute_store_dotdot_attributes.c)

target_include_directories(aoxpc_attribute_store
  PUBLIC
    include
    ${CMAKE_CURRENT_BINARY_DIR}/include/
  PRIVATE src)

target_link_libraries(
  aoxpc_attribute_store
  PUBLIC uic_attribute_store uic_definitions aoa_types
  PRIVATE uic_log uic_dotdot_mqtt aoxpc_unid)

# Mocks
#target_add_mock(aoxpc_attribute_store)

# Unit test
add_subdirectory(test)


