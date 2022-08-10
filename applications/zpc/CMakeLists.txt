if(NOT ZPC_SOURCE_DIR)
  set(ZPC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif(NOT ZPC_SOURCE_DIR)

add_subdirectory(components)
add_subdirectory(applications)

add_executable(zpc main.c)
target_link_libraries(
  zpc
  zpc_stdin
  zwave_controller
  zpc_datastore
  zwave_s2
  zwave_network_management
  zwave_smartstart_management
  zwave_transports
  zwave_command_handler
  zwave_command_classes
  network_monitor
  uic_attribute_store
  uic_attribute_utils
  uic_dotdot_attribute_store
  zpc_attribute_resolver
  zpc_attribute_store
  unify_attribute_poll
  dotdot_mapper
  dotdot_mqtt_topics_handler
  uic_main
  uic_attribute_mapper
  ucl_mqtt
  zcl_cluster_servers
  zpc_rust
  zpc_ncp_update)

target_include_directories(
  zpc
  PRIVATE ${PROJECT_SOURCE_DIR}/components/uic_rust/unify_middleware/include/)

target_compile_definitions(
  zpc PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

install(TARGETS zpc RUNTIME DESTINATION "bin" COMPONENT uic-zpc)

install(
  FILES scripts/bash-completion/zpc
  DESTINATION share/bash-completion/completions
  COMPONENT uic-zpc)

install(
  DIRECTORY components/dotdot_mapper/rules
  COMPONENT uic-zpc
  DESTINATION share/uic
  FILES_MATCHING
  PATTERN *.uam)

if( "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
    install(
    DIRECTORY
    DESTINATION /var/lib/zpc
    COMPONENT uic-zpc)

  install(
    FILES scripts/systemd/uic-zpc.service
    DESTINATION /lib/systemd/system/
    COMPONENT uic-zpc)

  install(
    FILES scripts/udev/99-leds.rules
    DESTINATION /etc/udev/rules.d/
    COMPONENT uic-zpc)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/config.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/config)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/templates.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/templates)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/conffiles.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/conffiles)

include(scripts/scripts.cmake)

add_component_to_uic(
  uic-zpc # Package Name
  "Universal IOT controller - Z-Wave protocol Controller"
  "${CMAKE_PROJECT_NAME}-zpc" # Package Debian filename
  "libuic" # Package Depends on
  "${CMAKE_BINARY_DIR}/applications/zpc/debconf/config;\
    ${CMAKE_BINARY_DIR}/applications/zpc/debconf/templates;\
    ${CMAKE_BINARY_DIR}/applications/zpc/debconf/conffiles;\
    ${CMAKE_BINARY_DIR}/applications/zpc/debconf/postinst;\
    ${CMAKE_BINARY_DIR}/applications/zpc/debconf/prerm")

