setup_binding(zwave_tx ALLOWLIST ".*zwave_tx.*|ZWAVE_TX.*" BLOCKLIST ".*zwave_controller.*")
setup_binding(zwave_controller ALLOWLIST ".*zwave_controller.*|zwave_node_.*")
setup_binding(zwave_command_classes ALLOWLIST ".*zwave_command_class.*|zwave_firmware.*" BLOCKLIST "zwave_controller.*" EXT_LIB uic_config uic_log uic_datastore uic_attribute_store uic_contiki_platform)
setup_binding(zwave_command_handler ALLOWLIST ".*zwave_command_handler.*" BLOCKLIST "zwave_controller.*")

