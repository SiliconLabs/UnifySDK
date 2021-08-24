target_add_unittest(
    zwave_controller
    NAME zwave_controller_callbacks_test
    SOURCES zwave_controller_callbacks_test.c
    DEPENDS
    zwave_rx_mock
    zwave_api_mock
    zwave_network_management_mock
    zwave_tx_mock
  )

target_add_unittest(
    zwave_controller
    NAME zwave_controller_transport_test
    SOURCES zwave_controller_transport_test.c
    DEPENDS zwave_controller_transport_test_mock zwave_tx_mock
)

add_mock(zwave_controller_transport_test_mock
         zwave_controller_transport_test_callbacks.h)
target_interface_libraries(zwave_controller_transport_test_mock zwave_rx zwave_controller)

target_add_unittest(zwave_controller
NAME zwave_controller_utils_test
SOURCES zwave_controller_utils_test.c
DEPENDS
    zpc_attribute_store_mock
    uic_attribute_store_mock
    zpc_utils_mock
    zwave_unid_mock
)
