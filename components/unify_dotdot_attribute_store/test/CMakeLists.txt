add_library(unify_dotdot_attribute_store_shared SHARED
            $<TARGET_OBJECTS:unify_dotdot_attribute_store>)

target_link_libraries(unify_dotdot_attribute_store_shared unify)

target_include_directories(
  unify_dotdot_attribute_store_shared
  PUBLIC ../$<TARGET_PROPERTY:unify_dotdot_attribute_store,INCLUDE_DIRECTORIES>
         ../src ../zap-generated/src)

target_add_unittest(
  unify_dotdot_attribute_store_shared
  NAME
  unify_dotdot_attribute_store_test
  SOURCES
  ../zap-generated/test/unify_dotdot_attribute_store_test.c
  unify_dotdot_attribute_store_command_mapper_barrier_control_test.c
  unify_dotdot_attribute_store_command_mapper_color_control_test.c
  unify_dotdot_attribute_store_command_mapper_level_test.c
  unify_dotdot_attribute_store_command_mapper_on_off_test.c
  unify_dotdot_attribute_store_command_mapper_door_lock_test.c
  unify_dotdot_attribute_store_command_mapper_identify_test.c
  unify_dotdot_attribute_store_command_mapper_poll_control_test.c
  unify_dotdot_attribute_store_command_mapper_thermostat_test.c
  unify_dotdot_attribute_store_command_mapper_window_covering_test.c
  unify_dotdot_attribute_store_command_mapper_zwave_fan_control_test.c

  DEPENDS
  uic_dotdot_mqtt_mock
  uic_mqtt_mock
  uic_contiki_stub
  EXCLUDE
  uic_dotdot_mqtt
  uic_mqtt)

target_include_directories(
  unify_dotdot_attribute_store_test
  PUBLIC ../src ../zap-generated/src ../zap-generated/test
         ../zap-generated/include)


target_add_unittest(
  unify_dotdot_attribute_store_shared
  NAME
  unify_dotdot_attribute_store_configuration_test
  SOURCES
  unify_dotdot_attribute_store_configuration_test.c
)

target_add_unittest(
  unify_dotdot_attribute_store_shared
  NAME
  unify_dotdot_attribute_store_node_state_test
  SOURCES
  unify_dotdot_attribute_store_node_state_test.c
  DEPENDS
  uic_mqtt_mock
  uic_dotdot_mqtt_mock
  uic_contiki_stub
)


target_add_unittest(
  unify_dotdot_attribute_store_shared
  NAME
  unify_dotdot_attribute_store_group_cluster_test
  SOURCES
  unify_dotdot_attribute_store_group_cluster_test.c
  DEPENDS
  uic_mqtt_mock
  uic_dotdot_mqtt_mock
  uic_contiki_stub
)

target_add_unittest(
  unify_dotdot_attribute_store_shared
  NAME
  unify_dotdot_attribute_store_descriptor_test
  SOURCES
  unify_dotdot_attribute_store_descriptor_test.c
  DEPENDS
  uic_mqtt_mock
  uic_dotdot_mqtt_mock
)
