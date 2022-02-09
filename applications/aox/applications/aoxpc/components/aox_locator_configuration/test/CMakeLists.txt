target_add_unittest(
  aox_locator_configuration
  NAME aox_locator_configuration_test
  SOURCES aox_locator_configuration_test.c
  DEPENDS
    uic_dotdot_mqtt_mock
    aoxpc_unid_mock
)


target_add_unittest(
  aox_locator_configuration
  NAME aox_locator_configuration_persistence_test
  SOURCES aox_locator_configuration_persistence_test.c
  DEPENDS
    aoxpc_unid_mock
)
