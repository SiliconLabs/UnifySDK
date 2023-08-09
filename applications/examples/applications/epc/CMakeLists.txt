add_subdirectory(components)

# Example Protocol Controller executable
add_executable(epc main.c)
target_link_libraries(epc PUBLIC unify epc_application_monitoring epc_config
                                 epc_datastore)

target_compile_definitions(
  epc PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
              CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")
