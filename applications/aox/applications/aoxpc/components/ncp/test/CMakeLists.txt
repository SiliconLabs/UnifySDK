# This executable performs HIL tests. Therefore, it's not suitable for unit testing.
# It requires special test binary on the NCP target side.

add_executable(ncp_test
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/example_host/ncp_test/ncp_test.c
  ncp_test_fixt.c
  main.c)

target_link_libraries(ncp_test uic_log uic_main ncp)

target_include_directories(ncp_test
  PRIVATE
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/example_host/ncp_test
  ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(
  ncp_test PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")
