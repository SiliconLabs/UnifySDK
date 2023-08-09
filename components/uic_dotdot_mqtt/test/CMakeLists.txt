set(ZAP_FILE_INCLUDE_PATH ${ZAP_CURRENT_OUTPUT_DIR}/test)
add_library(uic_dotdot_mqtt_shared SHARED $<TARGET_OBJECTS:uic_dotdot_mqtt>)
target_link_libraries(uic_dotdot_mqtt_shared unify uic_definitions)
target_include_directories(
  uic_dotdot_mqtt_shared
  PUBLIC $<TARGET_PROPERTY:uic_dotdot_mqtt,INCLUDE_DIRECTORIES>
         ${ZAP_FILE_INCLUDE_PATH})

# Dotdot MQTT test
target_add_unittest(
  uic_dotdot_mqtt_shared
  NAME
  uic_dotdot_mqtt_test
  SOURCES
  dotdot_mqtt_test.cpp
  DEPENDS
  uic_mqtt_stub)

target_add_unittest(
  uic_dotdot_mqtt_shared
  NAME
  dotdot_mqtt_attributes_test
  SOURCES
  dotdot_mqtt_attributes_test.cpp
  DEPENDS
  uic_mqtt_stub)

# Dotdot MQTT Send Commands test
target_add_unittest(
  uic_dotdot_mqtt_shared
  NAME
  dotdot_mqtt_send_commands_test
  SOURCES
  dotdot_mqtt_send_commands_test.cpp
  DEPENDS
  uic_mqtt_stub)

# Dotdot MQTT Generated Commands test
target_add_unittest(
  uic_dotdot_mqtt_shared
  NAME
  dotdot_mqtt_generated_commands_test
  SOURCES
  dotdot_mqtt_generated_commands_test.cpp
  DEPENDS
  uic_mqtt_stub)

target_add_unittest(
  uic_dotdot_mqtt_shared
  NAME
  dotdot_mqtt_parsing_helpers_test
  SOURCES
  dotdot_mqtt_parsing_helpers_test.cpp
  DEPENDS
  uic_mqtt_stub
  uic_definitions)

target_add_unittest(
  uic_dotdot_mqtt_shared
  NAME
  dotdot_mqtt_group_commands_test
  SOURCES
  dotdot_mqtt_group_commands_test.cpp
  DEPENDS
  uic_mqtt_stub)

target_add_unittest(
  uic_dotdot_mqtt_shared
  NAME
  dotdot_mqtt_supported_generated_commands_test
  SOURCES
  dotdot_mqtt_supported_generated_commands_test.cpp
  DEPENDS
  uic_mqtt_stub)

target_add_unittest(
  uic_dotdot_mqtt_shared
  NAME
  dotdot_mqtt_helpers_test
  SOURCES
  dotdot_mqtt_helpers_test.cpp
  DEPENDS
  uic_mqtt_stub)

# Test Additional Cluster/Attribute/Value names
target_add_unittest(
  uic_dotdot_mqtt_shared
  NAME
  uic_dotdot_mqtt_additional_value_names_test
  SOURCES
  uic_dotdot_mqtt_additional_value_names_test.cpp
  DEPENDS
  uic_dotdot)