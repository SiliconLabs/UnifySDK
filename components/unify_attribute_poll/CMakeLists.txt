cargo_build(MANIFEST_FILE Cargo.toml)

set_target_properties(
  unify_attribute_poll
  PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
             LINK_LIBRARIES uic_attribute_store)
install(
  FILES $<TARGET_PROPERTY:unify_attribute_poll,IMPORTED_LOCATION>
  DESTINATION lib
  COMPONENT libuic)

target_add_mock(unify_attribute_poll)
