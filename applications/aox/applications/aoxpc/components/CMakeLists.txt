add_subdirectory(aoxpc_unid)
add_subdirectory(aoxpc_datastore)
add_subdirectory(aoxpc_config)
add_subdirectory(aoxpc_attribute_store)
add_subdirectory(aox_locator_configuration)
add_subdirectory(aoxpc_correction)
add_subdirectory(cte)
add_subdirectory(cte_parse)
add_subdirectory(db)
add_subdirectory(ncp)

# ##############################################################################
# Bluetooth API component
# ##############################################################################
add_library(bluetooth_api INTERFACE)

target_include_directories(bluetooth_api
  INTERFACE
  ${GeckoSDK_ROOT_DIR}/protocol/bluetooth/inc
)
