run_zap(zap/gen-templates.json)

# Library
add_library(
  aox_locator_configuration
  src/aox_locator_configuration.c
  ${ZAP_CURRENT_OUTPUT_DIR}/src/aox_locator_configuration_attributes.c)

target_include_directories(
  aox_locator_configuration
  PUBLIC include ${ZAP_CURRENT_OUTPUT_DIR}/include/
  PRIVATE src)

target_link_libraries(
  aox_locator_configuration
  PUBLIC unify
  PRIVATE aoxpc_unid aoxpc_attribute_store)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
