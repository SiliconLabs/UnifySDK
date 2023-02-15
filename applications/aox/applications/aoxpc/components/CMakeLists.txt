add_subdirectory(aoxpc_unid)
add_subdirectory(aoxpc_datastore)
add_subdirectory(aoxpc_config)
add_subdirectory(aoxpc_attribute_store)
add_subdirectory(aox_locator_configuration)
add_subdirectory(aoxpc_application_monitoring)
add_subdirectory(aoxpc_correction)
add_subdirectory(aoxpc_ncp_core)
add_subdirectory(aoxpc_ncp_process)
add_subdirectory(aoxpc_ncp_update)

# Components from GSDK

# ##############################################################################
# AoA Constant Tone Extension (CTE) handle component
# ##############################################################################
add_library(cte
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_cte/aoa_cte.c
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_cte/cte_conn_less.c
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_cte/cte_conn.c
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_cte/cte_silabs.c
)
target_include_directories(cte
  PUBLIC ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_cte
  PRIVATE ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_cte/config
          ${GeckoSDK_ROOT_DIR}/app/bluetooth/common/ncp_evt_filter
)
# Make sure that the sl_status.h comes from the GSDK
target_include_directories(cte BEFORE
  PRIVATE ${GeckoSDK_ROOT_DIR}/platform/common/inc
)
target_link_libraries(cte bluetooth_api aoa_db aoa_util antenna_array app_log_wrapper)

# ##############################################################################
# AoA Constant Tone Extension (CTE) configuration parser component
# ##############################################################################
add_library(cte_parse
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_parse/aoa_parse_cte.c
)
target_include_directories(cte_parse
  PUBLIC ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_parse
)
target_link_libraries(cte_parse cte)

# ##############################################################################
# Asset tag database component
# ##############################################################################
add_library(aoa_db
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_db/aoa_db.c
)
target_include_directories(aoa_db
  PUBLIC ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_db
  PRIVATE ${GeckoSDK_ROOT_DIR}/platform/common/inc
)
target_link_libraries(aoa_db bluetooth_api)

# ##############################################################################
# Bluetooth API component
# ##############################################################################
add_library(bluetooth_api INTERFACE)

target_include_directories(bluetooth_api
  INTERFACE
  ${GeckoSDK_ROOT_DIR}/protocol/bluetooth/inc
)

# ##############################################################################
# UART Device Firmware Update component
# ##############################################################################

add_library(uart_dfu
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/uart_dfu/uart_dfu.c
)

target_include_directories(uart_dfu
  PUBLIC ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/uart_dfu
)

target_link_libraries(uart_dfu bluetooth_api)
