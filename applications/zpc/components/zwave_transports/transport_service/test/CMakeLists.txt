target_add_unittest(transport_service
SOURCES
  transport_service_test.c
DEPENDS
  transport_service_mock
  zwave_tx_scheme_selector_mock
)
