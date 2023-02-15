if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
target_add_unittest(zpc_attribute_resolver
SOURCES zpc_attribute_resolver_test.c
DEPENDS
  uic_attribute_store_mock
  uic_attribute_resolver_rule_mock
  uic_attribute_resolver_mock
  zwave_tx_scheme_selector_mock
  zwave_tx_mock
  zwave_tx_groups_mock
  zwave_controller_mock
  zwave_command_classes_mock
  zpc_attribute_store_mock
)
