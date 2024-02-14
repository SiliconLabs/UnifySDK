# Library
run_zap(zap/gen-templates.json)

add_library(
  unify_dotdot_attribute_store OBJECT
  src/unify_dotdot_attribute_store.c
  src/unify_dotdot_attribute_store_configuration.cpp
  src/unify_dotdot_attribute_store_command_callbacks.c
  src/unify_dotdot_attribute_store_command_callbacks_barrier_control.c
  src/unify_dotdot_attribute_store_command_callbacks_color_control.c
  src/unify_dotdot_attribute_store_command_callbacks_door_lock.c
  src/unify_dotdot_attribute_store_command_callbacks_identify.c
  src/unify_dotdot_attribute_store_command_callbacks_level.c
  src/unify_dotdot_attribute_store_command_callbacks_on_off.c
  src/unify_dotdot_attribute_store_command_callbacks_poll_control.c
  src/unify_dotdot_attribute_store_command_callbacks_thermostat.c
  src/unify_dotdot_attribute_store_command_callbacks_window_covering.c
  src/unify_dotdot_attribute_store_node_state.cpp
  src/unify_dotdot_attribute_store_group_cluster.cpp
  src/unify_dotdot_attribute_store_descriptor.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/unify_dotdot_attribute_store_helpers.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/unify_dotdot_attribute_store_registration.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/unify_dotdot_attribute_store_write_attributes_command_callbacks.c
  ${ZAP_CURRENT_OUTPUT_DIR}/src/unify_dotdot_attribute_store_force_read_attributes_command_callbacks.c
  ${ZAP_CURRENT_OUTPUT_DIR}/src/unify_dotdot_attribute_store_attribute_publisher.cpp
)

target_include_directories(
  unify_dotdot_attribute_store
  PRIVATE src ${ZAP_CURRENT_OUTPUT_DIR}/src
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
         $<BUILD_INTERFACE:${ZAP_CURRENT_OUTPUT_DIR}/include>)

target_link_libraries(
  unify_dotdot_attribute_store
  PUBLIC uic_definitions uic_dotdot uic_attribute_store uic_attribute_utils uic_contiki
  PRIVATE uic_dotdot_mqtt uic_log)

if(BUILD_TESTING)
  # Mock
  add_mock(
    unify_dotdot_attribute_store_mock include/unify_dotdot_attribute_store.h
    # ${ZAP_CURRENT_OUTPUT_DIR}/include/unify_dotdot_attribute_store_helpers.h
    # ${ZAP_CURRENT_OUTPUT_DIR}/include/unify_dotdot_defined_attribute_types.h
  )

  target_interface_libraries(unify_dotdot_attribute_store_mock unify)
  # Test
  add_subdirectory(test)
endif()

install_headers(unify_dotdot_attribute_store uic libuic-dev)
