add_library(aoxpc_ncp_update src/aoxpc_ncp_update.c)
target_include_directories(aoxpc_ncp_update PUBLIC include)
# Make sure that the sl_status.h comes from the GSDK
target_include_directories(aoxpc_ncp_update BEFORE
  PRIVATE ${GeckoSDK_ROOT_DIR}/platform/common/inc)
target_link_libraries(aoxpc_ncp_update
  PRIVATE uic_config uart_dfu aoxpc_config bluetooth_api uic_log)
