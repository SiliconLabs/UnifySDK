target_add_unittest(
    zigpc_binding
    NAME
    zigpc_binding_request_test
    SOURCES
    zigpc_binding_request_test.cpp
    EXCLUDE
    zigpc_common_zigbee.c
    DEPENDS
    zigpc_common_mock
    zigpc_datastore_mock
    zigpc_gateway_mock)

target_add_unittest(
    zigpc_binding
    NAME
    zigpc_binding_mqtt_test
    SOURCES
    zigpc_binding_mqtt_test.cpp
    DEPENDS
    zigpc_common_mock
    zigpc_datastore_mock
    zigpc_gateway_mock
    )


target_add_unittest(
    zigpc_binding
    NAME
    zigpc_binding_response_test
    SOURCES
    zigpc_binding_response_test.cpp
    DEPENDS
    zigpc_common_mock
    zigpc_gateway_mock
    zigpc_datastore_mock
    )
