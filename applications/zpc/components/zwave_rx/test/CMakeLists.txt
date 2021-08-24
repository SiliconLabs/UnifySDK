target_add_unittest(zwave_rx
SOURCES zwave_rx_test.c zwave_rx_test_zpc_config_mock.c
DEPENDS
  zwave_api_mock
  uic_main_fd_mock
  zwave_controller_mock
  uic_contiki_stub
)

target_include_directories(zwave_rx_test PRIVATE ../src)
