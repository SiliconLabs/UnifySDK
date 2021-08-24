target_add_unittest(zwave_security_validation
SOURCES
  zwave_security_validation_test.c
DEPENDS
  uic_attribute_store_mock
  zpc_attribute_store_mock
  zwave_controller_mock
)
