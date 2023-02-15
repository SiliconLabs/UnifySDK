if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
target_add_unittest(
  zigpc_ucl
  NAME
  zigpc_ucl_mqtt_test
  SOURCES
  zigpc_ucl_mqtt_test.cpp
  DEPENDS
  uic_mqtt_mock)

target_add_unittest(
  zigpc_ucl
  NAME
  zigpc_ucl_pc_nwmgmt_test
  SOURCES
  zigpc_ucl_pc_nwmgmt_test.cpp
  DEPENDS
  zigpc_datastore_mock
  zigpc_net_mgmt_mock
  uic_mqtt_mock)

target_add_unittest(
  zigpc_ucl
  NAME
  zigpc_ucl_node_state_test
  SOURCES
  zigpc_ucl_node_state_test.cpp
  DEPENDS
  zigpc_datastore_mock
  uic_mqtt_mock)
