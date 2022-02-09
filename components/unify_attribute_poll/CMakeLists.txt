# Attribute poll engine TODO Add interface library fo attribute poll
cargo_build(MANIFEST_FILE Cargo.toml)

set_target_properties(
  unify_attribute_poll_target PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                         ${CMAKE_CURRENT_SOURCE_DIR}/include)
