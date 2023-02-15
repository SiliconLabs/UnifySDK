add_executable(cpc-hci-bridge cpc-hci-bridge.c)

target_link_libraries(cpc-hci-bridge
  cpc
  unify
  util
  pthread
)

if(CPCD_LOCATION)
  target_include_directories(cpc-hci-bridge PRIVATE ${CPCD_LOCATION}/include)
  set_source_files_properties(
    cpc-hci-bridge.c PROPERTIES COMPILE_FLAGS -Wno-error=unused-result
  )
else()
  get_target_property(CPC_SOURCE_DIR cpc SOURCE_DIR)
  target_include_directories(cpc-hci-bridge PRIVATE ${CPC_SOURCE_DIR}/lib)
endif()

install(TARGETS cpc-hci-bridge RUNTIME DESTINATION "bin" COMPONENT uic-cpc-hci-bridge)

set(DEFAULT_CPC_HCI_BRIDGE_INSTALL_PATH /var/lib/cpc-hci-bridge)
if(NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/${DEFAULT_CPC_HCI_BRIDGE_INSTALL_PATH} COMPONENT uic-cpc-hci-bridge)
else()
  install(DIRECTORY DESTINATION ${DEFAULT_CPC_HCI_BRIDGE_INSTALL_PATH} COMPONENT uic-cpc-hci-bridge)
endif()

if(NOT APPLE)
  set(DEFAULT_CPC_HCI_BRIDGE_SERVICE_PATH /lib/systemd/system)
  if(NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CPC_HCI_BRIDGE_SERVICE_PATH ${CMAKE_INSTALL_PREFIX}${DEFAULT_CPC_HCI_BRIDGE_SERVICE_PATH})
  else()
    set(CPC_HCI_BRIDGE_SERVICE_PATH ${DEFAULT_CPC_HCI_BRIDGE_SERVICE_PATH})
  endif()
  
  message(STATUS "Installing cpc-hci-bridge service to ${CPC_HCI_BRIDGE_SERVICE_PATH}")
  
  install(
    FILES debconf/uic-cpc-hci-bridge.service
    DESTINATION ${CPC_HCI_BRIDGE_SERVICE_PATH}
    COMPONENT uic-cpc-hci-bridge)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)

add_component_to_uic(
  uic-cpc-hci-bridge
  "Universal IOT controller - cpc-hci-bridge"
  "${CMAKE_PROJECT_NAME}-cpc-hci-bridge"
  ""
  ""
  "${CMAKE_BINARY_DIR}/applications/aox/applications/cpc-hci-bridge/debconf/postinst;\
  ${CMAKE_BINARY_DIR}/applications/aox/applications/cpc-hci-bridge/debconf/prerm")
