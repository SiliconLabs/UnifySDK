target_add_unittest(zpc_attribute_resolver
SOURCES zpc_attribute_resolver_test.c
DEPENDS
  uic_attribute_store_mock
  uic_attribute_resolver_rule_mock
  uic_attribute_resolver_mock
  zwave_tx_scheme_selector_mock
  zwave_tx_mock
  zwave_tx_groups_mock
  zpc_utils_mock
  zwave_unid_mock
  zwave_controller_mock
  zwave_command_classes_mock
  zpc_attribute_store_mock
)
