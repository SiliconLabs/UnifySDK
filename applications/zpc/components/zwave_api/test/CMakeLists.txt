target_add_unittest(zwave_api
NAME zwapi_timestamp_test
SOURCES zwapi_timestamp_test.c
)

target_add_unittest(zwave_api
NAME zwapi_protocol_mem_mock_test
SOURCES zwapi_protocol_mem_mock_test.c
DEPENDS zwapi_protocol_mem_mock
EXCLUDE zwapi_protocol_mem.c
)

target_add_unittest(zwave_api
NAME zwapi_init_test
SOURCES zwapi_init_test.c
DEPENDS zwapi_internal_mock
        zwave_api_mock
EXCLUDE zwapi_session.c
        zwapi_protocol_basis.c
)

target_add_unittest(zwave_api
NAME zwapi_protocol_rx_dispatch_test
SOURCES zwapi_protocol_rx_dispatch_test.c
        zwapi_internal_init_mock.c
DEPENDS zwapi_internal_mock
        zwave_api_mock
EXCLUDE zwapi_init.c
        zwapi_session.c
)

target_add_unittest(zwave_api
NAME zwapi_protocol_basis_test
SOURCES zwapi_protocol_basis_test.c
        zwapi_internal_init_mock.c
DEPENDS zwapi_internal_mock
        zwave_api_mock
EXCLUDE zwapi_session.c
        zwapi_init.c
)

target_add_unittest(zwave_api
NAME zwapi_protocol_transport_test
SOURCES zwapi_protocol_transport_test.c
        zwapi_internal_init_mock.c
DEPENDS zwapi_internal_mock
        zwave_api_mock
EXCLUDE zwapi_connection.c
        zwapi_session.c
        zwapi_init.c
)
