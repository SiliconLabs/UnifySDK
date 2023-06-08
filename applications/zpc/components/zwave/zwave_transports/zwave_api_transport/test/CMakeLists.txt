# Unit Tests
target_add_unittest(zwave_api_transport
SOURCES zwave_api_transport_test.c
DEPENDS
  uic_contiki_stub
  zwave_api_mock
  zwave_tx_groups_mock
  zwave_controller_mock
  zwave_rx_mock
)
