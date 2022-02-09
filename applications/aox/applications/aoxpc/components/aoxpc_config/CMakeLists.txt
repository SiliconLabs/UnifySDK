# AoX Protocol Controller Configuration
add_library(aoxpc_config src/aoxpc_config.c)
target_include_directories(aoxpc_config PUBLIC include)
target_link_libraries(aoxpc_config PUBLIC uic_config)

# Test
add_subdirectory(test)

# mocks
target_add_mock(aoxpc_config)
