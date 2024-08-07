find_package(GeckoSDK 2024.6.0 REQUIRED)
find_package(CPCD 4.5.0 REQUIRED)

set(CPC_HCI_BRIDGE_SOURCE_FILES
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/example_host/bt_host_cpc_hci_bridge/main.c
  ${GeckoSDK_ROOT_DIR}/app/common/util/app_log/app_log.c
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/iostream_mock/sl_iostream_handles.c
)

set(CPC_HCI_BRIDGE_INCLUDES
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/example_host/bt_host_cpc_hci_bridge
  ${GeckoSDK_ROOT_DIR}/platform/common/inc
  ${GeckoSDK_ROOT_DIR}/protocol/bluetooth/bgstack/ll/utils/hci_packet/inc
  ${GeckoSDK_ROOT_DIR}/app/common/util/app_log
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/iostream_mock
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/app_log
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/app_log/config
)

add_executable(cpc-hci-bridge ${CPC_HCI_BRIDGE_SOURCE_FILES})

target_include_directories(cpc-hci-bridge BEFORE PRIVATE
  ${CPC_HCI_BRIDGE_INCLUDES}
  ${cpcd_SOURCE_DIR}/lib
)

target_compile_definitions(cpc-hci-bridge PRIVATE
  -DHOST_TOOLCHAIN
  -DPOSIX
)

target_link_libraries(cpc-hci-bridge
  cpc
  util
  pthread
)
target_link_directories(cpc-hci-bridge PRIVATE ${cpcd_BINARY_DIR})

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
  "${CMAKE_PROJECT_NAME}-cpcd"
  ""
  "${CMAKE_BINARY_DIR}/applications/aox/applications/cpc-hci-bridge/debconf/postinst;\
  ${CMAKE_BINARY_DIR}/applications/aox/applications/cpc-hci-bridge/debconf/prerm")
