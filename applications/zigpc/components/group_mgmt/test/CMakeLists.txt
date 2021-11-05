# Group Map Tests
target_add_unittest(zigpc_group_mgmt NAME zigpc_group_mgmt_map_test SOURCES
                    zigpc_group_mgmt_map_test.c)

add_mock(zigpc_group_map_mock ../src/zigpc_group_map.h)
target_interface_libraries(zigpc_group_map_mock zigpc_common_mock
                           zigpc_group_mgmt_mock)

# Group MQTT Tests
target_add_unittest(
  zigpc_group_mgmt
  NAME
  zigpc_group_mgmt_mqtt_test
  SOURCES
  zigpc_group_mgmt_mqtt_test.cpp
  EXCLUDE
  zigpc_group_map.cpp
  DEPENDS
  zigpc_group_map_mock
  zigpc_zcl_command_parser_mock)
