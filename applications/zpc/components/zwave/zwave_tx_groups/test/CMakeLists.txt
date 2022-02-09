# Unit Tests
target_add_unittest(zwave_tx_groups
NAME zwave_tx_group_test
SOURCES zwave_tx_groups_test.c
DEPENDS
  zwave_controller_mock
EXCLUDE
  zwave_tx_groups_test.c
)