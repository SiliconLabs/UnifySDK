# Autogeneration using ZAP
run_zap(zap/gen-templates.json)

add_library(dotdot_attributes
            ${CMAKE_CURRENT_BINARY_DIR}/src/dotdot_attributes.cpp)

target_include_directories(
  dotdot_attributes
  PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/include
  PRIVATE src)

target_link_libraries(dotdot_attributes uic_dotdot zpc_attribute_store)

# Binding cluster mapper helper
add_library(
  dotdot_mapper_binding_cluster_helper
  src/binding_cluster_mapper_helper.cpp)
target_include_directories(
  dotdot_mapper_binding_cluster_helper
  PUBLIC include
  PRIVATE src)
target_link_libraries(dotdot_mapper_binding_cluster_helper
  PUBLIC uic_definitions zwave_definitions PRIVATE zpc_attribute_store uic_dotdot)

# DotDot Cluster Attribute Mappers
add_library(
  dotdot_mapper
  src/basic_cluster_mapper.cpp
  src/door_lock_cluster_mapper.cpp
  src/dotdot_mapper_fixt.c
  src/identify_cluster_mapper.cpp
  src/level_cluster_mapper.cpp
  src/color_control_cluster_mapper.cpp
  src/on_off_cluster_mapper.cpp
  src/thermostat_cluster_mapper.cpp
  src/binding_cluster_mapper.c)

target_include_directories(
  dotdot_mapper
  PUBLIC include
  PRIVATE src test)

target_link_libraries(
  dotdot_mapper
  PUBLIC dotdot_mapper_binding_cluster_helper
         uic_dotdot_mqtt
         dotdot_attributes
         dotdot_mqtt_topics_handler
         zpc_attribute_store
  PRIVATE zwave_handlers zwave_controller ucl_definitions)

add_subdirectory(test)
