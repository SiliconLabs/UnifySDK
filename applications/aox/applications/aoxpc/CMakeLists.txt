add_subdirectory(components)

add_executable(aoxpc aoxpc.c aoxpc_configuration_handler.c main.c)

target_link_libraries(aoxpc
  aoxpc_correction
  aoxpc_attribute_store
  ucl_mqtt_helper
  antenna_array
  aoa_angle
  aoa_util
  aoa_parse
  cte_parse
  cte
  aoa_db
  aox_locator_configuration
  aoxpc_application_monitoring
  aoxpc_unid
  aoxpc_datastore
  aoxpc_config
  aoxpc_ncp_core
  aoxpc_ncp_process
  aoxpc_ncp_update
  unify
)

target_compile_definitions(
  aoxpc PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

install(TARGETS aoxpc RUNTIME DESTINATION "bin" COMPONENT uic-aoxpc)

set(DEFAULT_AOXPC_RUNTIME_PATH /var/lib/aoxpc)
if(NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(AOXPC_RUNTIME_PATH ${CMAKE_INSTALL_PREFIX}/${DEFAULT_AOXPC_RUNTIME_PATH})
else()  
  set(AOXPC_RUNTIME_PATH ${DEFAULT_AOXPC_RUNTIME_PATH})
endif()

install(DIRECTORY DESTINATION ${AOXPC_RUNTIME_PATH} COMPONENT uic-aoxpc)
message(STATUS "AOXPC runtime directory is ${AOXPC_RUNTIME_PATH}")

if(NOT APPLE)
  set(DEFAULT_AOXPC_SERVICE_PATH /lib/systemd/system/)
  if(NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(AOXPC_SERVICE_PATH ${CMAKE_INSTALL_PREFIX}/${DEFAULT_AOXPC_SERVICE_PATH})
  else()  
    set(AOXPC_SERVICE_PATH ${DEFAULT_AOXPC_SERVICE_PATH})
  endif()

  install(
    FILES debconf/uic-aoxpc.service
    DESTINATION ${AOXPC_SERVICE_PATH}
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
  "libunify"
  ""
  "${CMAKE_BINARY_DIR}/applications/aox/applications/aoxpc/debconf/postinst;\
  ${CMAKE_BINARY_DIR}/applications/aox/applications/aoxpc/debconf/prerm")
