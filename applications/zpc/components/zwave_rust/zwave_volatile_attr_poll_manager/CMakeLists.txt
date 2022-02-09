cargo_build(MANIFEST_FILE Cargo.toml)

set_target_properties(
  zwave_volatile_attr_poll_manager_target PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                         ${CMAKE_CURRENT_SOURCE_DIR}/include)