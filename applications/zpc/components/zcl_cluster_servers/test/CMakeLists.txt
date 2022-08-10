target_add_unittest(
  zcl_group_cluster_server
  SOURCES
  zcl_group_cluster_server_test.c
  DEPENDS
  uic_datastore
  uic_attribute_store
  uic_attribute_resolver
  zpc_attribute_store
  ucl_node_state
  uic_dotdot_attribute_store
  uic_mqtt_mock
  dotdot_mqtt_topics_handler_mock
  zwave_network_management_mock)

# ZCL OTA cluster server test
# UIC-1969: Reactivate this :-)
# target_add_unittest(
#   zcl_OTA_cluster_server
#   NAME zcl_OTA_cluster_server_test
#   SOURCES zcl_OTA_cluster_server_test.cpp
#   DEPENDS
#   uic_ota_mock
#   zwave_command_classes_mock
#   uic_mqtt_stub
# )
# target_include_directories(zcl_OTA_cluster_server_test PRIVATE ../src)

# ProtocolController RFTelemetry cluster server test
target_add_unittest(
  zcl_rf_telemetry_cluster_server
  SOURCES
    zcl_rf_telemetry_cluster_server_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  zwave_controller_mock
  zwave_network_management_mock
  uic_dotdot_mqtt_mock
  zwave_rx_mock
  zwave_api_mock
  zwave_api_transport_mock
)

# Configuration Parameter Cluster Server test
target_add_unittest(
  configuration_parameter_cluster_server
  SOURCES
    configuration_parameter_cluster_server_test.c
  DEPENDS
    zpc_attribute_store_test_helper
    zpc_attribute_store
    uic_dotdot_mqtt_mock
    zwave_network_management_mock
)

# Binding Cluster Server Mapper test
target_add_unittest(
  zcl_binding_cluster_server
  SOURCES
  zcl_binding_cluster_server_test.c
  DEPENDS
    zpc_attribute_store_test_helper
    uic_dotdot_mqtt_mock
    zwave_network_management_mock
)