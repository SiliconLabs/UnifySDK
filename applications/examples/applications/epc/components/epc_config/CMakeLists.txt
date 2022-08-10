
# Example Protocol Controller config
add_library(epc_config src/epc_config.c)

target_link_libraries(epc_config uic_log uic_config)
target_include_directories(epc_config PUBLIC include)
target_compile_definitions(epc_config
  PRIVATE
    CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
    CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}"
)

# Unit tests
add_subdirectory(test)

# mocks
target_add_mock(epc_config)
