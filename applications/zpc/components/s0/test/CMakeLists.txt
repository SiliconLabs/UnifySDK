target_add_unittest(zwave_s0
SOURCES
  zwave_s0_sm_test.c
DEPENDS
  zwave_network_management_mock
  zwave_tx_mock
  zwave_s0_mock
)
