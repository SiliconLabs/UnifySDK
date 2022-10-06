cargo_build(NAME "zpc_rust" DEPENDS unify)

set_target_properties(zpc_rust PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                          ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(zpc_rust PROPERTIES INTERFACE_LINK_LIBRARIES "pthread;dl")

install(
  FILES zwave_poll_config.yaml
  DESTINATION share/uic
  COMPONENT uic-zpc)
