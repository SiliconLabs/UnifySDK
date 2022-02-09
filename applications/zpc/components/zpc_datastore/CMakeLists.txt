# ZPC datastore library
add_library(zpc_datastore src/zpc_datastore.c)
target_link_libraries(zpc_datastore uic_datastore zpc_config)
target_include_directories(zpc_datastore PUBLIC include)

# mocks
target_add_mock(zpc_datastore)

# Test
add_subdirectory(test)