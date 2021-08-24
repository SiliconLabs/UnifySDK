target_add_unittest(network_monitor SOURCES network_monitor_test.c
DEPENDS
    uic_contiki_stub
    zwapi_protocol_mem_mock
    zwave_controller_mock
    zwave_network_management_mock
    zwave_api_mock
    uic_mqtt_mock
    zwave_tx_scheme_selector_mock
    zwave_command_classes_mock
    uic_attribute_store_mock
    zpc_attribute_store_mock
    uic_attribute_resolver_mock
	  zpc_utils_mock
	  zpc_config_mock
    ucl_mqtt_node_interview_mock
)
