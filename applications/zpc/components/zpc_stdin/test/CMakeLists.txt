# Unit Tests
target_add_unittest(zpc_stdin SOURCES zpc_stdin_test.c
DEPENDS
  zwave_network_management_mock
  zwave_tx_scheme_selector_mock
  uic_attribute_resolver_mock
  uic_attribute_store_mock
  zpc_attribute_store_mock
  zwave_controller_mock
  ucl_mqtt_node_interview_mock
  zwave_command_classes_mock
  zwave_command_handler_mock
  zwave_tx_mock
  zwave_unid_mock
  zwave_tx_groups_mock
  zwave_s2_mock
)
