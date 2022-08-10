set(ZWAVE_INCLUDE_DIR
    ${ZPC_SOURCE_DIR}/components/zwave/zwave_definitions/include)

# Z-Wave S2 Nonce management "sub-library"
add_library(zwave_s2_nonce_management src/zwave_s2_nonce_management.c)

target_include_directories(
  zwave_s2_nonce_management
  PUBLIC include)

target_link_libraries(zwave_s2_nonce_management
                      PUBLIC s2_controller
                      PRIVATE zwave_controller)
target_compile_definitions(zwave_s2_nonce_management PRIVATE ZW_CONTROLLER)


IF(ZWAVE_BUILD_SYSTEM EQUAL 1)
  SET(KEYSTORE_SOURCES "")
ELSE()
  SET(KEYSTORE_SOURCES "src/zwave_s2_keystore.c")
ENDIF()

# Z-Wave S2 main library
add_library(
  zwave_s2
  src/zwave_s2_fixt.c ${KEYSTORE_SOURCES} src/zwave_s2_network.c
  src/zwave_s2_printf.c src/zwave_s2_process.c src/zwave_s2_transport.c)

IF(ZWAVE_BUILD_SYSTEM EQUAL 1)
target_include_directories(
  zwave_s2
  PUBLIC include
  ${SUBTREE_UIC}/applications/zpc/components/zpc_utils/include/
  PRIVATE libs/zw-libs2/include src)
ELSE()
target_include_directories(
  zwave_s2
  PUBLIC include
  PRIVATE libs/zw-libs2/include src)
ENDIF()

install(TARGETS zwave_s2 LIBRARY DESTINATION lib)

IF(NOT ZWAVE_BUILD_SYSTEM EQUAL 1)
  add_subdirectory(libs/zw-libs2)
ENDIF()

target_link_libraries(
  zwave_s2
  PUBLIC zwave_s2_nonce_management
  PRIVATE s2_controller uic_contiki zwave_controller zwave_s0)

target_compile_definitions(zwave_s2 PRIVATE ZIPGW ZW_CONTROLLER)

target_add_mock(zwave_s2)
add_subdirectory(test)

add_mock(libs2_mock libs/zw-libs2/include/S2.h
         libs/zw-libs2/include/s2_inclusion.h
         libs/zw-libs2/include/s2_protocol.h)
target_interface_libraries(libs2_mock zwave_definitions s2crypto aes)
target_compile_definitions(libs2_mock PUBLIC ZIPGW)
