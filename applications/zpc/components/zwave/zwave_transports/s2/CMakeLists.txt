set(ZWAVE_INCLUDE_DIR
    ${ZPC_SOURCE_DIR}/components/zwave/zwave_definitions/include)

# Z-Wave S2 Nonce management "sub-library"
add_library(zwave_s2_nonce_management src/zwave_s2_nonce_management.c)

target_include_directories(
  zwave_s2_nonce_management
  PUBLIC include)

target_link_libraries(zwave_s2_nonce_management
                      PUBLIC s2_controller
                      PRIVATE zwave_definitions zwave_network_management)
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

target_include_directories(
  zwave_s2
  PUBLIC include
  PRIVATE libs/zw-libs2/include src)

install(TARGETS zwave_s2 LIBRARY DESTINATION lib)

if(ZWAVE_BUILD_SYSTEM EQUAL 1)

  target_include_directories(
    zwave_s2
    PUBLIC ${SUPER_DIR}/protocol/z-wave/Components/Assert
    PRIVATE
      ${SUBTREE_LIBS2}/include
      ${SUPER_DIR}/util/third_party/crypto/mbedtls/3rdparty/everest/include/everest/kremlin
  )

else() # UNIFY SECTION

  add_subdirectory(libs/zw-libs2)

endif() # End of differentiation

target_link_libraries(
  zwave_s2
  PUBLIC zwave_s2_nonce_management
  PRIVATE s2_controller uic_contiki zwave_definitions zwave_network_management
          zwave_rx zwave_controller)

target_compile_definitions(zwave_s2 PRIVATE ZIPGW ZW_CONTROLLER)

target_add_mock(zwave_s2)
add_subdirectory(test)

add_mock(libs2_mock libs/zw-libs2/include/S2.h
         libs/zw-libs2/include/s2_inclusion.h
         libs/zw-libs2/include/s2_protocol.h)
target_interface_libraries(libs2_mock zwave_definitions s2crypto aes)
target_compile_definitions(libs2_mock PUBLIC ZIPGW)
