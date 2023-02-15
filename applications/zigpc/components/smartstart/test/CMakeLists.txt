if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
# Smartstart Manager DSK Parsing Tests
target_add_unittest(
  zigpc_smartstart
  NAME
  zigpc_smartstart_dsk_test
  SOURCES
  zigpc_smartstart_dsk_test.cpp
  DEPENDS
  zigpc_gateway_mock)

# Smartstart Manager Interface Tests
target_add_unittest(
  zigpc_smartstart
  NAME
  zigpc_smartstart_test
  SOURCES
  zigpc_smartstart_test.cpp
  DEPENDS
  zigpc_gateway_mock
  zigpc_net_mgmt_mock
  zigpc_common
  uic_mqtt_mock
  uic_log
  uic_contiki_stub)
