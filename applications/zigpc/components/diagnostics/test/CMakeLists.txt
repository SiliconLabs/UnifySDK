set_source_files_properties(
  zigpc_diagnostics_test.cpp PROPERTIES COMPILE_FLAGS -Wno-format-security
)
  
target_add_unittest(zigpc_diagnostics SOURCES zigpc_diagnostics_test.cpp DEPENDS uic_mqtt_mock zigpc_gateway_mock)