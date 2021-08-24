set(ZWAVE_INCLUDE_DIR ${ZPC_SOURCE_DIR}/components/zwave_definitions/include)
add_subdirectory(libs/zw-libs2)

add_library(
  zwave_s2
  src/zwave_s2_fixt.c src/zwave_s2_keystore.c src/zwave_s2_network.c
  src/zwave_s2_printf.c src/zwave_s2_process.c src/zwave_s2_transport.c)

target_include_directories(
  zwave_s2
  PUBLIC include
  PRIVATE libs/zw-libs2/include src)

target_link_libraries(
  zwave_s2
  PRIVATE s2_controller uic_contiki zwave_definitions zwave_network_management zwave_rx zwave_controller uic_contiki_platform)

target_compile_definitions(zwave_s2 PRIVATE ZIPGW ZW_CONTROLLER)

target_add_mock(zwave_s2)
add_subdirectory(test)

add_mock(libs2_mock libs/zw-libs2/include/S2.h
libs/zw-libs2/include/s2_inclusion.h)
target_interface_libraries(libs2_mock zwave_definitions s2crypto aes)
target_compile_definitions(libs2_mock PUBLIC ZIPGW)
