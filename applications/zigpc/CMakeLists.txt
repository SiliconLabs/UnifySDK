find_package(GeckoSDK 4.1 REQUIRED)

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
  zigpc_discovery
  zigpc_zcl_profiles
  zigpc_gateway
  zigpc_net_mgmt
  zigpc_group_mgmt
  zigpc_ucl
  zigpc_uic_datastore_fixt
  zigpc_smartstart
  zigpc_stdin
  zigpc_zcl_command_parser
  zigpc_ota_zigbee
  zigpc_ncp_updater
  uic_attribute_store
  uic_datastore
  uic_dotdot_mqtt
  uic_main)

target_compile_definitions(
  zigpc PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

install(
    TARGETS zigpc 
    RUNTIME DESTINATION "bin" 
    PERMISSIONS
     OWNER_READ
     OWNER_WRITE
     OWNER_EXECUTE
     GROUP_READ
     GROUP_WRITE
     GROUP_EXECUTE
     WORLD_READ
     WORLD_EXECUTE
    COMPONENT uic-zigpc)

install(
  DIRECTORY
  DESTINATION /var/lib/zigpc
  COMPONENT uic-zigpc)

if(NOT APPLE)
  install(
    FILES scripts/systemd/uic-zigpc.service
    DESTINATION /lib/systemd/system/
    COMPONENT uic-zigpc)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/config.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/config)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postrm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postrm)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/templates.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/templates)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/conffiles.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/conffiles)

add_component_to_uic(
  uic-zigpc # Package Name
  "Universal IOT Controller - Zigbee Protocol Controller"
  "${CMAKE_PROJECT_NAME}-zigpc" # Package Debian filename
  "libuic" # Package Depends on
  "${CMAKE_BINARY_DIR}/applications/zigpc/debconf/config;\
    ${CMAKE_BINARY_DIR}/applications/zigpc/debconf/templates;\
    ${CMAKE_BINARY_DIR}/applications/zigpc/debconf/conffiles;\
    ${CMAKE_BINARY_DIR}/applications/zigpc/debconf/postinst;\
    ${CMAKE_BINARY_DIR}/applications/zigpc/debconf/prerm;\
    ${CMAKE_BINARY_DIR}/applications/zigpc/debconf/postrm")

