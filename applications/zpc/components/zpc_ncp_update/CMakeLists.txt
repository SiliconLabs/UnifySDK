add_library(zpc_ncp_update src/zpc_ncp_update_fixt.c)
target_include_directories(zpc_ncp_update PUBLIC include)
target_link_libraries(zpc_ncp_update PRIVATE zpc_config zwave_api
                                             uic_gbl_interface)

# Testing
target_add_unittest(
  zpc_ncp_update
  SOURCES
  test/zpc_ncp_update_test.c
  DEPENDS
  zpc_config_mock
  zwave_api_mock
  uic_gbl_interface_mock)
