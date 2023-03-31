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
  zigpc_datastore_ep_group_test
  SOURCES
  zigpc_datastore_ep_group_test.c
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

target_add_unittest(
  zigpc_datastore
  NAME
  zigpc_datastore_cluster_attr_test
  SOURCES
  zigpc_datastore_cluster_attr_test.c
  DEPENDS
  uic_attribute_store_mock)

target_add_unittest(
  zigpc_datastore
  NAME
  zigpc_datastore_cluster_cmd_test
  SOURCES
  zigpc_datastore_cluster_cmd_test.c
  DEPENDS
  uic_attribute_store_mock)

target_add_unittest(zigpc_datastore NAME zigpc_datastore_integration_test
                    SOURCES zigpc_datastore_integration_test.cpp)

target_add_unittest(
  zigpc_datastore
  NAME
  zigpc_datastore_util_test
  SOURCES
  zigpc_datastore_util_test.cpp
  EXCLUDE
  zigpc_datastore.cpp
  DEPENDS
  zigpc_datastore_mock)

#target_add_unittest(
#  zigpc_datastore
#  NAME
#  zigpc_datastore_binding_test
#  SOURCES
#  zigpc_datastore_binding_test.c
#  DEPENDS
#  uic_attribute_store_mock)
