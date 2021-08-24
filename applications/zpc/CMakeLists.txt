if(NOT ZPC_SOURCE_DIR)
  set(ZPC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif(NOT ZPC_SOURCE_DIR)

add_subdirectory(components)
add_subdirectory(applications)

add_executable(zpc main.c)
target_link_libraries(zpc
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
zpc_attribute_resolver
zpc_attribute_store
dotdot_mapper
dotdot_mqtt_topics_handler
uic_main
uic_attribute_mapper
ucl_mqtt
zcl_cluster_servers)

target_compile_definitions(
  zpc PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

set(description "Universal IOT controller - Z-wave protocol Controller")
cpack_add_component(zpc
                    DISPLAY_NAME "zpc"
                    DESCRIPTION ${description}
                    INSTALL_TYPES Full)

install(TARGETS zpc RUNTIME DESTINATION "bin" COMPONENT uic-zpc)
install(FILES scripts/bash-completion/zpc DESTINATION
        share/bash-completion/completions COMPONENT uic-zpc)
install(DIRECTORY components/dotdot_mapper/rules COMPONENT uic-zpc
        DESTINATION share/uic FILES_MATCHING PATTERN *.uam )
install(DIRECTORY DESTINATION /var/lib/zpc
        COMPONENT uic-zpc)

install(FILES scripts/systemd/uic-zpc.service DESTINATION
    /lib/systemd/system/ COMPONENT uic-zpc)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/config.in
  ${CMAKE_CURRENT_BINARY_DIR}/debconf/config)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst.in
  ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/templates.in
  ${CMAKE_CURRENT_BINARY_DIR}/debconf/templates)

include(scripts/scripts.cmake)
