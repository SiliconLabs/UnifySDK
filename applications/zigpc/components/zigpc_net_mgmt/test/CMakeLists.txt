if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
# Mocks
add_mock(zigpc_net_mgmt_process_send_mock ../src/zigpc_net_mgmt_process_send.h)
target_interface_libraries(zigpc_net_mgmt_process_send_mock zigpc_common_mock)

add_mock(zigpc_net_mgmt_notify_mock ../src/zigpc_net_mgmt_notify_int.h)
target_include_directories(zigpc_net_mgmt_notify_mock PRIVATE ../include)
target_interface_libraries(zigpc_net_mgmt_notify_mock 
                           zigpc_common_mock zigpc_discovery_mock zigpc_gateway_mock)

# Notify Tests
target_add_unittest(
  zigpc_net_mgmt
  NAME
  zigpc_net_mgmt_notify_test
  SOURCES
  zigpc_net_mgmt_notify_test.c
  DEPENDS
  zigpc_common_mock)

# Gateway Observer Tests
target_add_unittest(
  zigpc_net_mgmt
  NAME
  zigpc_net_mgmt_gateway_test
  SOURCES
  zigpc_net_mgmt_gateway_test.c
  EXCLUDE
  zigpc_net_mgmt_process.c
  zigpc_net_mgmt_fsm.c
  DEPENDS
  zigpc_net_mgmt_process_send_mock
  zigpc_discovery_mock
  zigpc_gateway_mock
  uic_log)

# Gateway API Tests
target_add_unittest(
  zigpc_net_mgmt
  NAME
  zigpc_net_mgmt_test
  SOURCES
  zigpc_net_mgmt_test.c
  EXCLUDE
  zigpc_net_mgmt_process.c
  zigpc_net_mgmt_fsm.c
  DEPENDS
  zigpc_net_mgmt_process_send_mock
  zigpc_gateway_mock
  zigpc_common_mock
  zigpc_datastore_mock)
