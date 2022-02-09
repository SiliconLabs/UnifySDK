# ZAP is required here
find_program(ZAP_EXECUTABLE "zap" REQUIRED)
run_zap(zap/gen-templates.json)

# Library
add_library(
  aox_locator_configuration
  src/aox_locator_configuration.c
  ${CMAKE_CURRENT_BINARY_DIR}/src/aox_locator_configuration_attributes.c
  )

target_include_directories(
  aox_locator_configuration
  PUBLIC
    include
    ${CMAKE_CURRENT_BINARY_DIR}/include/
  PRIVATE src)

target_link_libraries(
  aox_locator_configuration
  PUBLIC uic_log uic_definitions
  PRIVATE uic_dotdot_mqtt aoxpc_unid aoxpc_attribute_store)

add_subdirectory(test)
