# Mocks

add_mock(zigpc_net_mgmt_process_send_mock ../src/zigpc_net_mgmt_process_send.h)
target_interface_libraries(zigpc_net_mgmt_process_send_mock zigpc_common_mock)

add_mock(zigpc_net_mgmt_fsm_mock ../src/zigpc_net_mgmt_fsm.h)
target_include_directories(zigpc_net_mgmt_fsm_mock PRIVATE ../include)
target_interface_libraries(zigpc_net_mgmt_fsm_mock zigpc_common_mock
                           zigpc_gateway_mock uic_contiki_stub)

add_mock(zigpc_net_mgmt_notify_mock ../src/zigpc_net_mgmt_notify_int.h)
target_include_directories(zigpc_net_mgmt_notify_mock PRIVATE ../include)
target_interface_libraries(zigpc_net_mgmt_notify_mock zigpc_net_mgmt_fsm_mock
                           zigpc_common_mock zigpc_gateway_mock)

# FSM Log Tests
target_add_unittest(zigpc_net_mgmt NAME zigpc_net_mgmt_fsm_log_test SOURCES
                    zigpc_net_mgmt_fsm_log_test.c)

# FSM Tests
target_add_unittest(
  zigpc_net_mgmt
  NAME
  zigpc_net_mgmt_fsm_test
  SOURCES
  zigpc_net_mgmt_fsm_test.c
  DEPENDS
  zigpc_net_mgmt_notify_mock
  zigpc_gateway_mock)

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
  zigpc_net_mgmt_fsm_mock
  zigpc_common_mock
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
  zigpc_net_mgmt_fsm_mock
  zigpc_common_mock)
