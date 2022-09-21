add_mock(ucl_network_management_mock ../src/ucl_network_management.h)
target_link_libraries(ucl_network_management_mock PUBLIC zwave_definitions)

target_add_unittest(
  ucl_network_management
  SOURCES
  ucl_network_management_test.cpp
  DEPENDS
  uic_contiki_stub
  zwave_controller_mock
  zwave_network_management_mock
  uic_mqtt_stub
  ucl_nm_neighbor_discovery_mock)

target_add_unittest(
  ucl_node_state
  SOURCES
  ucl_node_state_test.c
  DEPENDS
  ucl_mqtt_node_interview_mock
  uic_contiki_stub
  ucl_network_management_mock
  uic_attribute_store_mock
  zpc_attribute_store_mock
  uic_mqtt_mock
  uic_dotdot_mqtt_mock)

target_add_unittest(
  ucl_mqtt_node_interview SOURCES ucl_mqtt_node_interview_test.c DEPENDS
  uic_attribute_store_mock zpc_attribute_store_mock)

target_add_unittest(
  ucl_nm_neighbor_discovery
  SOURCES
  ucl_nm_neighbor_discovery_test.c
  DEPENDS
  uic_contiki_stub
  zwave_controller_mock
  zwave_network_management_mock
  zpc_attribute_store_mock
  uic_attribute_store_mock
  uic_attribute_resolver_mock
)
