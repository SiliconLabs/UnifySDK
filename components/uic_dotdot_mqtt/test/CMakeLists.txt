# Dotdot MQTT test
target_add_unittest(
  uic_dotdot_mqtt
  NAME
  uic_dotdot_mqtt_test
  SOURCES
  dotdot_mqtt_test.cpp
  DEPENDS
  uic_mqtt_stub)

target_include_directories(uic_dotdot_mqtt_test
                           PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Dotdot MQTT Attributes test
target_add_unittest(
  uic_dotdot_mqtt
  NAME
  dotdot_mqtt_attributes_test
  SOURCES
  dotdot_mqtt_attributes_test.cpp
  DEPENDS
  uic_mqtt_stub)

target_include_directories(dotdot_mqtt_attributes_test
                           PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Dotdot MQTT Send Commands test
target_add_unittest(
  uic_dotdot_mqtt
  NAME
  dotdot_mqtt_send_commands_test
  SOURCES
  dotdot_mqtt_send_commands_test.cpp
  DEPENDS
  uic_mqtt_stub)

target_include_directories(dotdot_mqtt_send_commands_test
                           PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Dotdot MQTT Generated Commands test
target_add_unittest(
  uic_dotdot_mqtt
  NAME
  dotdot_mqtt_generated_commands_test
  SOURCES
  dotdot_mqtt_generated_commands_test.cpp
  DEPENDS
  uic_mqtt_stub)

target_include_directories(dotdot_mqtt_generated_commands_test
                          PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_add_unittest(
  uic_dotdot_mqtt
  NAME
  dotdot_mqtt_internals_test
  SOURCES
  dotdot_mqtt_internals_test.cpp
  DEPENDS
  uic_mqtt_stub)

target_add_unittest(
  uic_dotdot_mqtt
  NAME
  dotdot_mqtt_group_commands_test
  SOURCES
  dotdot_mqtt_group_commands_test.cpp
  DEPENDS
  uic_mqtt_stub)

target_add_unittest(
  uic_dotdot_mqtt
  NAME
  dotdot_mqtt_supported_generated_commands_test
  SOURCES
  dotdot_mqtt_supported_generated_commands_test.cpp
  DEPENDS
  uic_mqtt_stub)

target_add_unittest(
  uic_dotdot_mqtt
  NAME
  dotdot_mqtt_helpers_test
  SOURCES
  dotdot_mqtt_helpers_test.cpp
  DEPENDS
  uic_mqtt_stub)
