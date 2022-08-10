# EPC datastore library
add_library(epc_datastore src/epc_datastore.c)
target_link_libraries(epc_datastore uic_datastore epc_config)
target_include_directories(epc_datastore PUBLIC include)

# mocks
target_add_mock(epc_datastore)

# Test
add_subdirectory(test)