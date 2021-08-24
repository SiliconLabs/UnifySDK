# Group Map Tests
target_add_unittest(
  zigpc_group_mgmt
  NAME
  zigpc_group_mgmt_map_test
  SOURCES
  zigpc_group_mgmt_map_test.c
  DEPENDS
  zigpc_zcl_command_parser_mock)
