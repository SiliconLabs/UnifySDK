# AoXPC datastore library
add_library(aoxpc_datastore src/aoxpc_datastore.c)
target_link_libraries(aoxpc_datastore uic_datastore aoxpc_config)
target_include_directories(aoxpc_datastore PUBLIC include)

# mocks
target_add_mock(aoxpc_datastore)

# Test
add_subdirectory(test)