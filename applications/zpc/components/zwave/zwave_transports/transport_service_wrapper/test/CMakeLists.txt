target_add_unittest(
  transport_service_wrapper
  NAME zwave_transport_service_wrapper_test
  SOURCES zwave_transport_service_wrapper_test.c
  DEPENDS
    transport_service_mock
    zwave_controller_mock
    zwave_tx_scheme_selector_mock
    zwave_tx_mock
)
