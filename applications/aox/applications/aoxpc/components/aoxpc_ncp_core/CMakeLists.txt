# Network Co-Processor core module
add_library(aoxpc_ncp_core
  src/aoxpc_ncp_core.c
  ${GeckoSDK_ROOT_DIR}/protocol/bluetooth/src/sl_bt_ncp_host.c
  ${GeckoSDK_ROOT_DIR}/protocol/bluetooth/src/sl_bt_ncp_host_api.c
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/uart/uart_posix.c
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/tcp/tcp_posix.c
)

target_include_directories(aoxpc_ncp_core
  PUBLIC
  include
  PRIVATE
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/uart
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/tcp
)

target_link_libraries(aoxpc_ncp_core uic_log aoxpc_config bluetooth_api)
