if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
# Unit Tests
target_add_unittest(
  zwave_s2
  NAME
  zwave_s2_keystore_test
  SOURCES
  zwave_s2_keystore_test.c
  DEPENDS
  zwapi_protocol_mem_mock
  s2_inclusion_mocks
  zwave_api_mock
  zwave_controller_mock
  EXCLUDE
  S2.c)

target_add_unittest(
  zwave_s2
  NAME
  zwave_s2_test
  SOURCES
  zwave_s2_process_test.c
  DEPENDS
  libs2_mock
  uic_contiki_stub
  zwave_controller_mock)

# ########################## S2 Network test ##################################
add_mock(zwave_s2_network_callback_mock zwave_s2_network_callbacks.h)
target_interface_libraries(zwave_s2_network_callback_mock zwave_controller)

target_add_unittest(
  zwave_s2
  NAME
  zwave_s2_network_test
  SOURCES
  zwave_s2_network_test.c
  DEPENDS
  libs2_mock
  zwave_network_management_mock
  zwave_api_mock
  uic_contiki_stub
  zwave_controller_mock
  zwave_s2_network_callback_mock)

# ####################### S2 Transport test ##################################
target_add_unittest(
  zwave_s2
  NAME
  zwave_s2_transport_test
  SOURCES
  zwave_s2_transport_test.c
  DEPENDS
  zwave_controller ## Non-mock, we use the real thing here
  libs2_mock
  zwave_api_mock
  uic_contiki_stub
  zwave_tx_mock
  zwave_network_management_mock)

# ####################### S2 Nonce Management test ##################################
target_add_unittest(
  zwave_s2_nonce_management
  NAME
  zwave_s2_nonce_management_test
  SOURCES
  zwave_s2_nonce_management_test.c
  DEPENDS
  zwave_network_management_mock)
target_compile_definitions(zwave_s2_nonce_management_test PRIVATE ZW_CONTROLLER)
