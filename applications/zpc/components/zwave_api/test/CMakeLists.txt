target_add_unittest(zwave_api
NAME zwapi_timestamp_test
SOURCES zwapi_timestamp_test.c
)

target_add_unittest(zwave_api
NAME zwapi_protocol_mem_mock_test
SOURCES zwapi_protocol_mem_mock_test.c
DEPENDS zwapi_protocol_mem_mock
EXCLUDE src/zwapi_protocol_mem.c
)

target_add_unittest(zwave_api
NAME zwapi_init_test
SOURCES zwapi_init_test.c
DEPENDS zwapi_internal_mock
EXCLUDE src/zwapi_session.c
)

target_add_unittest(zwave_api
NAME zwapi_protocol_rx_dispatch_test
SOURCES zwapi_protocol_rx_dispatch_test.c
DEPENDS zwapi_internal_mock
EXCLUDE src/zwapi_session.c
)

target_add_unittest(zwave_api
NAME zwapi_protocol_basis_test
SOURCES zwapi_protocol_basis_test.c
DEPENDS zwapi_internal_mock
EXCLUDE src/zwapi_session.c
        src/zwapi_init.c
)
