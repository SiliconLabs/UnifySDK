setup_binding(uic_log ALLOWLIST ".*log.*")
setup_binding(uic_config ALLOWLIST "conf.*")
setup_binding(uic_attribute_store ALLOWLIST "attribute_store_.*|ATTRIBUTE_STORE.*" EXT_LIB uic_attribute_resolver)
setup_binding(uic_mqtt ALLOWLIST ".*mqtt_.*")
