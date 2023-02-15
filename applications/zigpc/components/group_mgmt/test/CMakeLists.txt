if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
# Group Map Tests
target_add_unittest(
  zigpc_group_mgmt
  NAME
  zigpc_group_mgmt_map_test
  SOURCES
  zigpc_group_mgmt_map_test.c
  DEPENDS
  zigpc_datastore_mock)

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
