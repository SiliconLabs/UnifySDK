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
  dotdot_attributes
  uic_mqtt_mock
  dotdot_mqtt_topics_handler_mock
  zwave_network_management_mock)

# ZCL OTA cluster server test
target_add_unittest(
  zcl_OTA_cluster_server
  SOURCES
  zcl_OTA_cluster_server_test.cpp
  DEPENDS
  uic_ota_mock
  zwave_command_classes_mock
  uic_mqtt_stub
)

# ProtocolController RFTelemetry cluster server test
target_add_unittest(
  zcl_rf_telemetry_cluster_server
  SOURCES
    zcl_rf_telemetry_cluster_server_test.c
  DEPENDS
  uic_datastore
  uic_attribute_store
  zpc_attribute_store_test_helper
  zwave_controller_mock
  zwave_network_management_mock
  uic_dotdot_mqtt_mock
)

# Configuration Parameter Cluster Cluster Mapper test
target_add_unittest(
  configuration_parameter_cluster_server
  SOURCES
    configuration_parameter_cluster_server_test.cpp
  DEPENDS
    zpc_attribute_store_test_helper
    uic_attribute_store
    zpc_attribute_store
    uic_dotdot_mqtt_mock
)