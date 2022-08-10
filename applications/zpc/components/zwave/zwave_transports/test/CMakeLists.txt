# Z-Wave transport fixt test
target_add_unittest(zwave_transports
SOURCES zwave_transports_fixt_test.c
DEPENDS
  zwave_s0_mock
)
