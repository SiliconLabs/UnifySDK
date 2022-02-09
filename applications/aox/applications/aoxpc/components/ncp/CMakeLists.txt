# Bluetooth Network Co-Processor host module
add_library(ncp
  src/ncp.c
  ${GeckoSDK_ROOT_DIR}/protocol/bluetooth/src/sl_bt_ncp_host.c
  ${GeckoSDK_ROOT_DIR}/protocol/bluetooth/src/sl_bt_ncp_host_api.c
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/uart/uart_posix.c
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/tcp/tcp_posix.c
)

target_include_directories(ncp
  PUBLIC
  include
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common/ncp_evt_filter
  PRIVATE
  ${GeckoSDK_ROOT_DIR}/platform/common/inc
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/uart
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/tcp
)

target_link_libraries(ncp uic_log aoxpc_config uic_main bluetooth_api)

option(BUILD_NCP_TEST "NCP HIL test")

if(BUILD_NCP_TEST MATCHES ON)
  add_subdirectory(test)
endif()
