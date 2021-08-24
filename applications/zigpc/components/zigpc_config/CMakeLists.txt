add_library(zigpc_config src/zigpc_config.c)
target_include_directories(zigpc_config PUBLIC include)
target_link_libraries(zigpc_config PRIVATE uic_config)

# Testing
add_subdirectory(test)

# Mocks
target_add_mock(zigpc_config)
