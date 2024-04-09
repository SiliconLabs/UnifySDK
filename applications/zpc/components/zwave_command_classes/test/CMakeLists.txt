if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
set(MOCK_LIBS
    uic_attribute_store_mock
    zpc_attribute_store_mock
    zwave_command_handler_mock
    zwave_controller_mock
    uic_attribute_resolver_mock
    uic_attribute_resolver_rule_mock
    zpc_config_mock
    zwave_tx_scheme_selector_mock
    zwave_tx_mock
    zwave_network_management_mock
    zwave_api_mock
    zwave_unid_mock)

add_library(zwave_command_classes_test $<TARGET_OBJECTS:zwave_command_classes>)
target_link_libraries(zwave_command_classes_test PUBLIC zwave_command_classes)

add_library(zwave_command_class_test_helper zwave_command_class_test_helper.c)
target_link_libraries(
  zwave_command_class_test_helper
  zwave_command_classes
  unity2
  zwave_tx_scheme_selector_mock
  zwave_tx_mock
  uic_attribute_store_mock
  zpc_attribute_store_mock)

target_add_unittest(zwave_command_classes NAME zwave_command_classes_fixt_test
                    SOURCES zwave_command_classes_fixt_test.c)

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_time_test
  SOURCES
  zwave_command_class_time_test.c
  DEPENDS
  zwave_command_class_test_helper
  ${MOCK_LIBS})

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_powerlevel_test
  SOURCES
  zwave_command_class_powerlevel_test.c
  DEPENDS
  zwave_command_class_test_helper
  uic_contiki_stub
  ${MOCK_LIBS})

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_manufacturer_specific_test
  SOURCES
  zwave_command_class_manufacturer_specific_test.c
  zwave_command_class_manufacturer_specific_test_zpc_config_mock.c
  DEPENDS
  zwave_command_class_test_helper
  zwave_security_validation_mock
  ${MOCK_LIBS})

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_s0_test
  SOURCES
  zwave_command_class_security_0_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zpc_utils
  zwave_controller
  zwave_command_handler_mock
  zwave_tx_scheme_selector_mock)

target_add_unittest(
  zwave_command_classes_test
  NAME
  zwave_command_class_indicator_test
  SOURCES
  zwave_command_class_indicator_test.c
  DEPENDS
  zwave_command_class_test_helper
  uic_attribute_store_mock
  zwave_command_handler_mock
  zwave_controller_mock
  zwave_tx_mock
  EXCLUDE
  platform_exec.c)

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_notification_test
  SOURCES
  zwave_command_class_notification_test.c
  DEPENDS
  ${MOCK_LIBS}
  zwave_command_class_test_helper)

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_supervision_test
  SOURCES
  zwave_command_class_supervision_test.c
  DEPENDS
  uic_contiki_stub
  zwave_tx_groups
  ${MOCK_LIBS})

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_supervision_test_no_mock
  SOURCES
  zwave_command_class_supervision_test_no_mock.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  uic_dotdot_mqtt_mock
)

# Wake Up Command Class
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_wake_up_test
  SOURCES
  zwave_command_class_wake_up_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zpc_utils
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  uic_attribute_utils_mock
  zpc_attribute_resolver_mock
  zpc_config_mock
  zwave_network_management_mock
  zwave_tx_scheme_selector_mock
  zwave_tx_mock
  EXCLUDE
  zpc_config.c)

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_security_2_test
  SOURCES
  zwave_command_class_security_2_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  zwave_tx_scheme_selector_mock)

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_version_test
  SOURCES
  zwave_command_class_version_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zpc_config_mock
  zwave_tx_scheme_selector_mock
  zwave_tx_mock)

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_node_info_resolver_test
  SOURCES
  zwave_command_class_node_info_resolver_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zpc_utils
  zwave_controller_mock
  uic_attribute_resolver_mock
  zwave_network_management_mock)

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_granted_keys_resolver_test
  SOURCES
  zwave_command_class_granted_keys_resolver_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock)

target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_zwaveplus_info_test
  SOURCES
  zwave_command_class_zwaveplus_info_test.c
  DEPENDS
  zwave_command_class_test_helper
  uic_contiki_stub
  ${MOCK_LIBS})

# Multi Channel test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_multi_channel_test
  SOURCES
  zwave_command_class_multi_channel_test.c
  DEPENDS
  ${MOCK_LIBS}
  EXCLUDE
  zwave_command_class_multi_channel.c)

# Multilevel Sensor test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_multilevel_sensor_test
  SOURCES
  zwave_command_class_multilevel_sensor_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock)

# Basic test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_basic_test
  SOURCES
  zwave_command_class_basic_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zpc_config_mock
  zwave_tx_scheme_selector_mock
  zwave_tx_mock
  uic_dotdot_mqtt_mock)

# Device Reset Locally test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_device_reset_locally_test
  SOURCES
  zwave_command_class_device_reset_locally_test.c
  DEPENDS
  uic_contiki_stub
  zwave_controller_mock
  zwave_command_handler_mock
  zwave_command_classes_mock
  zwave_network_management_mock
  EXCLUDE
  zwave_command_class_association_send.cpp)

# Z-Wave Command Class utils test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_classes_utils_test
  SOURCES
  zwave_command_classes_utils_test.c
  DEPENDS
  uic_attribute_store_mock
  zwave_controller_mock
  zwave_tx_scheme_selector_mock
  zpc_attribute_store_mock
  zwave_tx_mock
  zwave_unid_mock)

# Z-Wave Command Class generic tests with real attribute store.
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_classes_generic_tests
  SOURCES
  zwave_command_classes_generic_tests.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_command_handler_mock
  zwave_controller_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zpc_config_mock
  zwave_tx_scheme_selector_mock
  zwave_tx_mock)

# Central Scene Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_central_scene_test
  SOURCES
  zwave_command_class_central_scene_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  uic_attribute_utils_mock
  zwave_command_handler_mock
  zwave_controller_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zpc_config_mock
  zwave_tx_scheme_selector_mock
  zwave_tx_mock
  uic_dotdot_mqtt_mock)

# Configuration Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_configuration_control_test
  SOURCES
  zwave_command_class_configuration_control_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zwave_network_management_mock)

# Door Lock Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_door_lock_test
  SOURCES
  zwave_command_class_door_lock_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
)

# Alarm Sensor Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_alarm_sensor_test
  SOURCES
  zwave_command_class_alarm_sensor_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zpc_utils
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zwave_network_management_mock)

  # Barrier Operator Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_barrier_operator_test
  SOURCES
  zwave_command_class_barrier_operator_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zpc_utils
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zwave_network_management_mock
  uic_dotdot_mqtt_mock)

# Meter Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_meter_control_test
  SOURCES
  zwave_command_class_meter_control_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zwave_network_management_mock)

# Z-Wave Multilevel Switch Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_switch_multilevel_test
  SOURCES
  zwave_command_class_switch_multilevel_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  uic_attribute_utils_mock
  zwave_command_handler_mock
  zwave_controller_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zpc_config_mock
  zwave_tx_scheme_selector_mock
  zwave_tx_mock
  uic_dotdot_mqtt_mock)

# Z-Wave Color Switch Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_switch_color_test
  SOURCES
  zwave_command_class_switch_color_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  uic_attribute_utils_mock
  zwave_command_handler_mock
  zwave_controller_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zpc_config_mock
  zwave_tx_scheme_selector_mock
  zwave_tx_mock)

# Z-Wave Binary Switch Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_binary_switch_test
  SOURCES
  zwave_command_class_binary_switch_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_command_handler_mock
  zwave_controller_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zpc_config_mock
  zwave_tx_scheme_selector_mock
  zwave_tx_mock
  uic_dotdot_mqtt_mock)

# Z-Wave Firmware Update functionality test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_firmware_update_test
  SOURCES
  zwave_command_class_firmware_update_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  uic_ota
  zwave_tx_scheme_selector
  zwave_controller
  zwave_command_handler_mock
  zwave_network_management_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zpc_config_mock
  zwave_api_mock)

# Association
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_association_test
  SOURCES
  zwave_command_class_association_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  uic_contiki_stub
  zwave_controller_mock
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zwave_tx_scheme_selector_mock
  zwave_network_management_mock
  zwave_tx_mock)

# Multi Channel Association
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_multi_channel_association_test
  SOURCES
  zwave_command_class_multi_channel_association_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zwave_tx_scheme_selector_mock
  zwave_network_management_mock
  zwave_tx_mock)

# Association send module
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_association_send_test
  SOURCES
  zwave_command_class_association_send_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler
  uic_attribute_resolver
  zwave_tx_scheme_selector_mock
  zwave_network_management_mock
  zwave_tx_mock)

# Multi Command
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_multi_command_test
  SOURCES
  zwave_command_class_multi_command_test.c
  DEPENDS
  zpc_utils
  zwave_controller
  zwave_command_handler_mock
  zwave_network_management_mock)

# Command Class AGI test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_agi_test
  SOURCES
  zwave_command_class_agi_test.cpp
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  uic_attribute_resolver_mock
  zwave_command_handler_mock
  zwave_network_management_mock
  zwave_tx_mock)

# Command Class ICCC test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_inclusion_controller_test
  SOURCES
  zwave_command_class_inclusion_controller_test.cpp
  DEPENDS
  uic_contiki_stub
  zpc_attribute_store_test_helper
  zwave_command_classes
  uic_attribute_resolver_mock
  zwave_command_handler_mock
  zwave_controller_mock
  zwave_network_management_mock)

# Scene Activation Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_scene_activation_test
  SOURCES
  zwave_command_class_scene_activation_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  uic_dotdot_mqtt_mock)

# Scene Controller Configuration Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_scene_controller_configuration_test
  SOURCES
  zwave_command_class_scene_controller_configuration_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  zwave_network_management_mock)

# Scene Actuator Configuration Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_scene_actuator_configuration_test
  SOURCES
  zwave_command_class_scene_actuator_configuration_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_command_handler_mock
  uic_dotdot_mqtt_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock)

# Thermostat mode test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_thermostat_mode_test
  SOURCES
  zwave_command_class_thermostat_mode_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  uic_dotdot_mqtt_mock)

# Thermostat setpoint test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_thermostat_setpoint_test
  SOURCES
  zwave_command_class_thermostat_setpoint_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock)

# Sound switch unit test
target_add_unittest(zwave_command_classes
NAME zwave_command_class_sound_switch_test
SOURCES zwave_command_class_sound_switch_test.c
DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  uic_dotdot_mqtt_mock
)

# Thermostat fan mode test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_thermostat_fan_mode_test
  SOURCES
  zwave_command_class_thermostat_fan_mode_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  uic_dotdot_mqtt_mock)

  # Thermostat fan state test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_thermostat_fan_state_test
  SOURCES
  zwave_command_class_thermostat_fan_state_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  uic_dotdot_mqtt_mock)

# Thermostat operating state test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_thermostat_operating_state_test
  SOURCES
  zwave_command_class_thermostat_operating_state_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  uic_dotdot_mqtt_mock)

# Tests for generated command classes
set(GEN_TEST_INCLUDES
    "${CMAKE_SOURCE_DIR}/applications/zpc/components/zwave_controller/include"
    "${CMAKE_SOURCE_DIR}/include")

# Generated Command Class Door Lock test
target_add_unittest(
  zwave_command_classes
  NAME
  generated_door_lock_test
  SOURCES
  generated_door_lock_test.cpp
  DEPENDS
  uic_log
  zwave_handlers
  ${MOCK_LIBS})
target_include_directories(generated_door_lock_test
                           PRIVATE ${GEN_TEST_INCLUDES})
target_compile_options(generated_door_lock_test PRIVATE -Wno-unused-variable
                                                        -Wno-unused-function)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(generated_door_lock_test
                         PRIVATE -Wno-unused-but-set-variable)
endif()

# Generated Command Class Basic Tariff Info test
target_add_unittest(
  zwave_command_classes
  NAME
  generated_basic_tariff_info_test
  SOURCES
  generated_basic_tariff_info_test.cpp
  DEPENDS
  uic_log
  zwave_handlers
  ${MOCK_LIBS})
target_include_directories(generated_basic_tariff_info_test
                           PRIVATE ${GEN_TEST_INCLUDES})
target_compile_options(generated_basic_tariff_info_test
                       PRIVATE -Wno-unused-variable -Wno-unused-function)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(generated_basic_tariff_info_test
                         PRIVATE -Wno-unused-but-set-variable)
endif()

# Class Indicator control test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_indicator_control_test
  SOURCES
  zwave_command_class_indicator_control_test.cpp
  DEPENDS
  uic_log
  zwave_handlers
  uic_attribute_store
  zwave_network_management_mock
  uic_attribute_resolver_mock)

# Battery Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_battery_test
  SOURCES
  zwave_command_class_battery_test.cpp
  DEPENDS
  uic_log
  zwave_handlers
  uic_attribute_store
  zpc_attribute_store_test_helper)


# User Code Command Class test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_user_code_test
  SOURCES
  zwave_command_class_user_code_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  )
  
# Humidity Control Mode test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_humidity_control_mode_test
  SOURCES
  zwave_command_class_humidity_control_mode_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  uic_dotdot_mqtt_mock)

  # Humidity Control Operating State test
target_add_unittest(
  zwave_command_classes
  NAME
  zwave_command_class_humidity_control_operating_state_test
  SOURCES
  zwave_command_class_humidity_control_operating_state_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller
  zwave_command_handler_mock
  uic_attribute_resolver_mock
  zpc_attribute_resolver_mock
  uic_dotdot_mqtt_mock)

  # Humidity Control SetPoint test
  target_add_unittest(
    zwave_command_classes
    NAME
    zwave_command_class_humidity_control_setpoint_test
    SOURCES
    zwave_command_class_humidity_control_setpoint_test.cpp
    DEPENDS
    zpc_attribute_store_test_helper
    zwave_controller
    zwave_command_handler_mock
    uic_attribute_resolver_mock
    zpc_attribute_resolver_mock
    uic_dotdot_mqtt_mock)
  
  

  # User credential test
  target_add_unittest(
    zwave_command_classes
    NAME
    zwave_command_class_user_credential_test
    SOURCES
    zwave_command_class_user_credential_test.cpp
    DEPENDS
    zpc_attribute_store_test_helper
    zwave_controller
    zwave_command_handler_mock
    uic_attribute_resolver_mock
    zpc_attribute_resolver_mock
    uic_dotdot_mqtt_mock
    zwave_network_management_mock)