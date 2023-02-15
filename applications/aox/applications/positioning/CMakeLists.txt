add_subdirectory(components)

set(POSITIONING_SRC
  main.c
  positioning.c
  positioning_configuration_handler.c
)

add_executable(positioning ${POSITIONING_SRC})

target_link_libraries(positioning
  ucl_mqtt_helper
  aoa_parse
  aoa_util
  aoa_cjson
  angle_queue
  positioning_id
  positioning_application_monitoring
  loc
  unify
)

target_compile_definitions(
  positioning PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

install(TARGETS positioning RUNTIME DESTINATION "bin" COMPONENT uic-positioning)

set(DEFAULT_POSITIONING_RUNTIME_PATH /var/lib/positioning)
if(NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(POSITIONING_RUNTIME_PATH ${CMAKE_INSTALL_PREFIX}/${DEFAULT_POSITIONING_RUNTIME_PATH})
else()  
  set(POSITIONING_RUNTIME_PATH ${DEFAULT_POSITIONING_RUNTIME_PATH})
endif()

install(DIRECTORY DESTINATION ${POSITIONING_RUNTIME_PATH} COMPONENT uic-positioning)
message(STATUS "POSITIONING runtime directory is ${POSITIONING_RUNTIME_PATH}")

if(NOT APPLE)
  set(DEFAULT_POSITIONING_SERVICE_PATH /lib/systemd/system/)
  if(NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(POSITIONING_SERVICE_PATH ${CMAKE_INSTALL_PREFIX}/${DEFAULT_POSITIONING_SERVICE_PATH})
  else()  
    set(POSITIONING_SERVICE_PATH ${DEFAULT_POSITIONING_SERVICE_PATH})
  endif()

  install(
    FILES debconf/uic-positioning.service
    DESTINATION ${POSITIONING_SERVICE_PATH}
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
  "libunify"
  ""
  "${CMAKE_BINARY_DIR}/applications/aox/applications/positioning/debconf/postinst;\
  ${CMAKE_BINARY_DIR}/applications/aox/applications/positioning/debconf/prerm")

