# Unit Tests
target_add_unittest(
  zwave_tx
  SOURCES
  zwave_tx_test.c
  DEPENDS
  zwave_controller_mock
  uic_contiki_stub
  zwave_s2_mock
  zwave_network_management_mock
  zwave_tx_groups_mock)

target_add_unittest(zwave_tx NAME priority_queue_test SOURCES
                    priority_queue_test.cpp)

target_add_unittest(zwave_tx
  NAME zwave_tx_incoming_frames_test
  SOURCES zwave_tx_incoming_frames_test.cpp)
