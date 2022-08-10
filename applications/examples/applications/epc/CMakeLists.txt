add_subdirectory(components)

## Example Protocol Contoller executable
add_executable(epc main.c)
target_link_libraries(epc
  # Unify components
  uic_log
  uic_stdin
  uic_main
  uic_datastore
  uic_attribute_store
  uic_dotdot_attribute_store
  uic_dotdot_mqtt
  # EPC components
  epc_config
  epc_datastore
)

target_compile_definitions(epc
  PRIVATE
    CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
    CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}"
)
