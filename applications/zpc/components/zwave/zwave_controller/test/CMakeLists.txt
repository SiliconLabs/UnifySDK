target_add_unittest(
  zwave_controller
  NAME
  zwave_controller_test
  SOURCES
  zwave_controller_test.c
  DEPENDS
  zwave_api_mock
  zpc_config_mock
  zwave_s2_mock
  zwave_s0_mock)

target_add_unittest(
  zwave_controller
  NAME
  zwave_controller_keyset_test
  SOURCES
  zwave_controller_keyset_test.c
)

target_add_unittest(
  zwave_controller
  NAME
  zwave_controller_callbacks_reset_steps
  SOURCES
  zwave_controller_callbacks_reset_steps.c
  DEPENDS
  zwave_rx_mock
  zwave_api_mock
  zwave_network_management_mock
  zwave_tx_mock)

target_add_unittest(
  zwave_controller
  NAME
  zwave_controller_callbacks_test
  SOURCES
  zwave_controller_callbacks_test.c
  DEPENDS
  zwave_rx_mock
  zwave_api_mock
  zwave_network_management_mock
  zwave_tx_mock)

target_add_unittest(
  zwave_controller
  NAME
  zwave_controller_transport_test
  SOURCES
  zwave_controller_transport_test.c
  DEPENDS
  zwave_controller_transport_test_mock
  zwave_tx_mock)

add_mock(zwave_controller_transport_test_mock
         zwave_controller_transport_test_callbacks.h)
target_interface_libraries(zwave_controller_transport_test_mock zwave_rx
                           zwave_controller)

target_add_unittest(zwave_controller
  NAME    zwave_controller_utils_test
  SOURCES zwave_controller_utils_test.c
  DEPENDS zwave_tx_mock
)

target_add_unittest(zwave_controller NAME zwave_controller_crc16_test SOURCES
                    zwave_controller_crc16_test.c)
