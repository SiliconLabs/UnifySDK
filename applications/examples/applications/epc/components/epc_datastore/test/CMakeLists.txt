target_add_unittest(
  epc_datastore
  NAME
  epc_datastore_test
  SOURCES
  epc_datastore_test.c
  DEPENDS
  epc_config_mock
  uic_datastore_mock)
