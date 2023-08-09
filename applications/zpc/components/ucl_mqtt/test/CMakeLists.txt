# DISABLED target_add_unittest( ucl_mqtt NAME ucl_network_management_test
# SOURCES ucl_network_management_test.cpp DEPENDS uic_contiki_stub
# zwave_controller_mock zwave_network_management_mock uic_mqtt_stub
# ucl_nm_neighbor_discovery_mock EXCLUDE ucl_nm_neighbor_discovery.cpp)
# target_include_directories(ucl_network_management_test PRIVATE ../src)

target_add_unittest(ucl_mqtt SOURCES ucl_mqtt_node_interview_test.c DEPENDS
                    zpc_attribute_store_test_helper)

target_add_unittest(
  ucl_mqtt
  NAME
  ucl_nm_neighbor_discovery_test
  SOURCES
  ucl_nm_neighbor_discovery_test.c
  DEPENDS
  uic_contiki_stub
  zwave_controller_mock
  zwave_network_management_mock
  zpc_attribute_store_mock
  uic_attribute_store_mock
  uic_attribute_resolver_mock)
target_include_directories(ucl_nm_neighbor_discovery_test PRIVATE ../src)


target_add_unittest(
  ucl_mqtt
  NAME zpc_node_state_test
  SOURCES zpc_node_state_test.c
  DEPENDS
    zpc_attribute_store_test_helper
    uic_dotdot_mqtt_mock
    zwave_network_management_mock
)
target_include_directories(zpc_node_state_test PRIVATE ../src)
