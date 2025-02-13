set(CMAKE_CXX_STANDARD 17)  # Adjust as per your project requirements
set(CMAKE_CXX_STANDARD_REQUIRED ON)

run_zap(zap/gen-templates.json)

# EED attribute store Library
add_library(
  eed_attribute_store
  src/eed_attribute_store.c
  src/eed_attribute_store_helper.c
  src/eed_attribute_store_type_registration.cpp
  src/eed_cluster_configuration.cpp
  src/eed_matter_device_type.cpp
  src/eed_command_handler.cpp
  src/eed_attribute_custom_handler.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/eed_attribute_store_clusters.c
  ${ZAP_CURRENT_OUTPUT_DIR}/src/eed_dotdot_create_clusters.cpp)

target_include_directories(
  eed_attribute_store
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  eed_attribute_store
  PUBLIC unify)

