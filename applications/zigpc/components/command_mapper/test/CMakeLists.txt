if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
target_add_unittest(
  zigpc_command_mapper
  SOURCES
  zigpc_command_mapper_test.c
  DEPENDS
  zigpc_common_mock
  zigpc_zcl_util_mock
  zigpc_datastore_mock
  zigpc_gateway_mock)

target_add_unittest(
  zigpc_command_mapper
  NAME
  zigpc_command_mapper_multicast_test
  SOURCES
  zigpc_command_mapper_multicast_test.c
  DEPENDS
  zigpc_common_mock
  zigpc_zcl_util_mock
  zigpc_group_mgmt_mock
  zigpc_gateway_mock)

target_add_unittest(
  zigpc_command_mapper
  NAME
  zigpc_command_gen_cmd_publish_test
  SOURCES
  zigpc_command_gen_cmd_publish_test.cpp
  DEPENDS
  zigpc_zcl_command_parser_mock
  uic_dotdot_mqtt_mock)

target_add_unittest(
  zigpc_command_mapper
  NAME
  zigpc_cmdmapper_gen_unid_handlers_test
  SOURCES
  zigpc_cmdmapper_gen_unid_handlers_test.cpp
  DEPENDS
  zigpc_zcl_util_mock
  zigpc_gateway_mock)

target_add_unittest(
  zigpc_command_mapper
  NAME
  zigpc_cmdmapper_gen_bygroup_handlers_test
  SOURCES
  zigpc_cmdmapper_gen_bygroup_handlers_test.cpp
  DEPENDS
  zigpc_zcl_util_mock
  zigpc_group_mgmt_mock
  zigpc_gateway_mock)
