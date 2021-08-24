target_add_unittest(zwave_command_handler SOURCES
  zwave_command_handler_test.c
  command_class_handlers.c
DEPENDS
  zwave_controller_mock
  zwave_network_management_mock
  zwave_security_validation_mock
)
