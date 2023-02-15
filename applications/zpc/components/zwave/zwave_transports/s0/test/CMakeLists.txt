target_add_unittest(zwave_s0
NAME zwave_s0_sm_test
SOURCES
  zwave_s0_sm_test.c
DEPENDS
  zwave_network_management_mock
  zwave_tx_mock
  zwave_s0_transport_mock
)

target_add_unittest(zwave_s0
NAME zwave_s0_transport_test
SOURCES zwave_s0_transport_test.c
DEPENDS
  zwave_controller_mock
  zwave_tx_mock
  zwave_s0_mock
  zwave_network_management_mock
  uic_contiki_stub
EXCLUDE zwave_s0_network.c zwave_s0_sm.c
)
target_include_directories(zwave_s0_transport_test PRIVATE ../src)
target_compile_definitions(zwave_s0_transport_test PRIVATE ZIPGW ZW_CONTROLLER)
