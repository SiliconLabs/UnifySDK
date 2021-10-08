add_subdirectory(components)
add_subdirectory(applications)

add_executable(zigpc main.c)

target_link_libraries(
  zigpc
  zigpc_attribute_management
  zigpc_command_mapper
  zigpc_config
  zigpc_common
  zigpc_controller
  zigpc_diagnostics
  zigpc_zcl_profiles
  zigpc_gateway
  zigpc_net_mgmt
  zigpc_group_mgmt
  zigpc_node_state
  zigpc_ucl
  zigpc_smartstart
  zigpc_stdin
  zigpc_zcl_command_parser
  zigpc_ota_zigbee
  uic_attribute_store
  uic_datastore
  uic_dotdot_mqtt
  uic_main)

target_compile_definitions(
  zigpc PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

set(description "Universal IOT Controller - Zigbee Protocol Controller")

cpack_add_component(
  zigpc
  DISPLAY_NAME "zigpc"
  DESCRIPTION ${description}
  INSTALL_TYPES Full)
install(TARGETS zigpc RUNTIME DESTINATION "bin" COMPONENT uic-zigpc)
install(
  DIRECTORY
  DESTINATION ${CMAKE_INSTALL_PREFIX}/var/lib/zigpc
  COMPONENT uic-zigpc)
