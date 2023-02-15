cargo_build(NAME zpc_rust_build MANIFEST_PATH ${CMAKE_CURRENT_LIST_DIR}
            DISABLE_TESTS)
cargo_as_cmake_target(zpc_rust_build zpc_rust)

install(
  FILES zwave_poll_config.yaml
  DESTINATION share/uic
  COMPONENT uic-zpc)
