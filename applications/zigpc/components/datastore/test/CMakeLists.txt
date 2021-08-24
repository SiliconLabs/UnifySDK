target_add_unittest(
  zigpc_datastore
  NAME
  zigpc_datastore_nwk_test
  SOURCES
  zigpc_datastore_nwk_test.c
  DEPENDS
  uic_attribute_store_mock)

target_add_unittest(
  zigpc_datastore
  NAME
  zigpc_datastore_dev_test
  SOURCES
  zigpc_datastore_dev_test.c
  DEPENDS
  uic_attribute_store_mock)

target_add_unittest(
  zigpc_datastore
  NAME
  zigpc_datastore_ep_test
  SOURCES
  zigpc_datastore_ep_test.c
  DEPENDS
  uic_attribute_store_mock)

target_add_unittest(
  zigpc_datastore
  NAME
  zigpc_datastore_cluster_test
  SOURCES
  zigpc_datastore_cluster_test.c
  DEPENDS
  uic_attribute_store_mock)
