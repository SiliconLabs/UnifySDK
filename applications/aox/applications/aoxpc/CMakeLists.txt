add_subdirectory(components)

add_executable(aoxpc aoxpc.c aoxpc_configuration_handler.c main.c)

target_link_libraries(aoxpc
  aoxpc_correction
  aoxpc_attribute_store
  ucl_mqtt_helper
  uic_log
  uic_main
  antenna_array
  aoa_angle
  aoa_util
  aoa_parse
  cte_parse
  cte
  aoa_db
  uic_dotdot_mqtt
  aox_locator_configuration
  aoxpc_unid
  aoxpc_datastore
  aoxpc_config
  aoxpc_ncp_core
  aoxpc_ncp_process
  aoxpc_ncp_update
  uic_attribute_store
  uic_datastore
)

target_compile_definitions(
  aoxpc PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

install(TARGETS aoxpc RUNTIME DESTINATION "bin" COMPONENT uic-aoxpc)
install(DIRECTORY DESTINATION /var/lib/aoxpc COMPONENT uic-aoxpc)

if(NOT APPLE)
  install(
    FILES debconf/uic-aoxpc.service
    DESTINATION /lib/systemd/system/
    COMPONENT uic-aoxpc)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)

add_component_to_uic(
  uic-aoxpc
  "Universal IOT controller - AoX protocol Controller"
  "${CMAKE_PROJECT_NAME}-aoxpc"
  "libuic"
  "${CMAKE_BINARY_DIR}/applications/aox/applications/aoxpc/debconf/postinst;\
  ${CMAKE_BINARY_DIR}/applications/aox/applications/aoxpc/debconf/prerm")
