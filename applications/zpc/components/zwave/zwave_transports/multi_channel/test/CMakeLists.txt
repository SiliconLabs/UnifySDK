# Multi Channel transport test
target_add_unittest(zwave_multi_channel
SOURCES zwave_multi_channel_transport_test.c 
DEPENDS
  zwave_controller_mock
  zwave_tx_mock
)
