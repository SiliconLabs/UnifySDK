if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
target_add_unittest(
  zwave_rx
  SOURCES
  zwave_rx_test.c
  zwave_rx_test_zpc_config_mock.c
  DEPENDS
  zwave_api_mock
  uic_main_fd_mock
  zwave_controller_mock
  uic_contiki_stub)

target_include_directories(zwave_rx_test PRIVATE ../src)

target_add_unittest(
  zwave_rx
  NAME
  zwave_rx_zwapi_callback_test
  SOURCES
  zwave_rx_zwapi_callback_test.c
  DEPENDS
  zwave_api_mock)
target_include_directories(zwave_rx_zwapi_callback_test PRIVATE ../src)

target_add_unittest(
  zwave_rx
  NAME
  zwave_rx_callbacks_test
  SOURCES
  zwave_rx_callbacks_test.c
  DEPENDS
  zwave_api_mock)
target_include_directories(zwave_rx_callbacks_test PRIVATE ../src)
