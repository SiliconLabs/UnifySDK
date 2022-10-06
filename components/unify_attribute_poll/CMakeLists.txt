cargo_build(NAME "unify_attribute_poll")

# cargo build creates an imported target based on configuration defined in the
# rust manifest. append the include directories to this target so that it will
# be picked up by the install_headers macro.
set_target_properties(
  unify_attribute_poll
  PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
             LINK_LIBRARIES uic_attribute_store)
install_headers(unify_attribute_poll "uic" "uic-dev")

# make sure the shared library produced by rustc is also installed. its
# installed along side the unfiy library. which is fine as long as we are using
# them as static libraries
install(
  FILES $<TARGET_PROPERTY:unify_attribute_poll,IMPORTED_LOCATION>
  DESTINATION lib
  COMPONENT libuic)

target_add_mock(unify_attribute_poll)
