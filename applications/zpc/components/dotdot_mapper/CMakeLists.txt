# Autogeneration using ZAP
run_zap(zap/gen-templates.json)

add_library(dotdot_attributes
            ${CMAKE_CURRENT_BINARY_DIR}/src/dotdot_attributes.cpp)

target_include_directories(
  dotdot_attributes
  PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/include
  PRIVATE src)

target_link_libraries(dotdot_attributes zpc_utils uic_dotdot_types)

# DotDot Cluster Attribute Mappers
add_library(
  dotdot_mapper
  src/door_lock_cluster_mapper.cpp src/dotdot_mapper_fixt.c
  src/level_cluster_mapper.cpp src/on_off_cluster_mapper.cpp
  src/thermostat_cluster_mapper.cpp)

target_include_directories(
  dotdot_mapper
  PUBLIC include
  PRIVATE src test)

target_link_libraries(dotdot_mapper uic_dotdot_mqtt dotdot_attributes
                      dotdot_mqtt_topics_handler)

add_subdirectory(test)
