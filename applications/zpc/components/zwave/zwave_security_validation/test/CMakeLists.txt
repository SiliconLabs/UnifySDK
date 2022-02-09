target_add_unittest(zwave_security_validation
SOURCES
  zwave_security_validation_test.c
DEPENDS
  zwave_controller_mock
  zwave_tx_scheme_selector_mock
)
