add_subdirectory(components)

add_executable(aoxpc aoxpc.c aoxpc_configuration_handler.c main.c)

target_link_libraries(aoxpc
  aoxpc_correction
  ucl_mqtt_helper
  uic_log
  uic_main
  ncp
  aoa_angle
  aoa_utils
  aoa_parse
  cte_parse
  cte
  db
  uic_dotdot_mqtt
  aox_locator_configuration
  aoxpc_unid
  aoxpc_datastore
  aoxpc_config
  uic_attribute_store
  uic_datastore
)

target_compile_definitions(
  aoxpc PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

set(description "Universal IOT controller - AoX protocol Controller")
cpack_add_component(aoxpc
                    DISPLAY_NAME "aoxpc"
                    DESCRIPTION ${description}
                    INSTALL_TYPES Full)

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
