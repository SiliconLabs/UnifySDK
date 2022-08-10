add_subdirectory(components)

set(POSITIONING_SRC
  main.c
  positioning.c
  positioning_configuration_handler.c
)

add_executable(positioning ${POSITIONING_SRC})

target_link_libraries(positioning
  uic_log
  uic_main
  ucl_mqtt_helper
  aoa_parse
  aoa_util
  cjson
  angle_queue
  positioning_id
  loc
  uic_dotdot_mqtt
)

target_compile_definitions(
  positioning PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

install(TARGETS positioning RUNTIME DESTINATION "bin" COMPONENT uic-positioning)
install(DIRECTORY DESTINATION /var/lib/positioning COMPONENT uic-positioning)

if(NOT APPLE)
  install(
    FILES debconf/uic-positioning.service
    DESTINATION /lib/systemd/system/
    COMPONENT uic-positioning)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)

add_component_to_uic(
  uic-positioning
  "Universal IOT controller - Positioning Service"
  "${CMAKE_PROJECT_NAME}-positioning"
  "libuic"
  "${CMAKE_BINARY_DIR}/applications/aox/applications/positioning/debconf/postinst;\
  ${CMAKE_BINARY_DIR}/applications/aox/applications/positioning/debconf/prerm")

