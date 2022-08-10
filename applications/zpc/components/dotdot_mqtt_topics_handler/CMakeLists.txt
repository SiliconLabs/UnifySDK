# Autogeneration using ZAP
run_zap(zap/gen-templates.json)

add_library(
  dotdot_mqtt_topics_handler
  ${CMAKE_CURRENT_BINARY_DIR}/src/dotdot_mqtt_topics_handler.cpp
  src/zpc_dotdot_mqtt.c src/zpc_dotdot_mqtt_group_dispatch.cpp)

target_include_directories(
  dotdot_mqtt_topics_handler
  PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/include/
  PRIVATE src)

target_link_libraries(
  dotdot_mqtt_topics_handler
  PUBLIC dotdot_attributes zpc_attribute_store uic_attribute_store
  PRIVATE zcl_cluster_servers uic_mqtt uic_dotdot_mqtt zwave_controller
          ucl_definitions uic_attribute_mapper uic_attribute_utils)

target_compile_options(dotdot_mqtt_topics_handler PRIVATE -Wno-unused-variable
                                                          -Wno-unused-function)
# Unit tests
add_subdirectory(test)

# Mocks
target_add_mock(dotdot_mqtt_topics_handler)
