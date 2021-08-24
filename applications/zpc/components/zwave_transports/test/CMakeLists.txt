# Multi Channel transport test
target_add_unittest(zwave_transports
SOURCES zwave_multi_channel_transport_test.c 
DEPENDS
  zwave_controller_mock
  zwave_tx_mock
)
target_add_unittest(zwave_transports
NAME zwave_transport_service_wrapper_test
SOURCES zwave_transport_service_wrapper_test.c 
DEPENDS
  transport_service_mock
  zwave_controller_mock
)

target_add_unittest(zwave_transports
NAME zwave_s0_transport_test
SOURCES zwave_s0_transport_test.c
DEPENDS
  zwave_controller_mock
  zwave_s0_mock
  zwave_tx_mock
  zwave_tx_scheme_selector_mock
  zwave_network_management_mock
)
