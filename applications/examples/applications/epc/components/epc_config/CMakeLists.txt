# Example Protocol Controller config
add_library(epc_config src/epc_config.c)

target_link_libraries(epc_config PRIVATE unify uic_log)
target_include_directories(epc_config PUBLIC include)
target_compile_definitions(
  epc_config PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
                     CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

if(BUILD_TESTING)
  # Unit tests
  add_subdirectory(test)

  # mocks
  target_add_mock(epc_config)
endif()
