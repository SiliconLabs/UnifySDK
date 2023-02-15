set(ZWAVE_INCLUDE_DIR
    ${ZPC_SOURCE_DIR}/components/zwave/zwave_definitions/include)

# Z-Wave S2 Nonce management "sub-library"
add_library(zwave_s2_nonce_management src/zwave_s2_nonce_management.c)

target_include_directories(zwave_s2_nonce_management PUBLIC include)

target_link_libraries(
  zwave_s2_nonce_management
  PUBLIC s2_controller
  PRIVATE zwave_controller)
target_compile_definitions(zwave_s2_nonce_management PRIVATE ZW_CONTROLLER)

if(ZWAVE_BUILD_SYSTEM EQUAL 1)
  set(KEYSTORE_SOURCES "")
else()
  set(KEYSTORE_SOURCES "src/zwave_s2_keystore.c")
endif()

# Z-Wave S2 main library
add_library(
  zwave_s2
  src/zwave_s2_fixt.c ${KEYSTORE_SOURCES} src/zwave_s2_network.c
  src/zwave_s2_printf.c src/zwave_s2_process.c src/zwave_s2_transport.c)

set(DEFAULT_ZW-LIBS2_PATH libs/zw-libs2)
if(EXISTS ${ZW-LIBS2_LOCATION})
  set(ZW-LIBS2_PATH ${ZW-LIBS2_LOCATION})
  message(STATUS "Found ZW-LIBS2: ${ZW-LIBS2_LOCATION}")
  add_subdirectory(${ZW-LIBS2_PATH} ${CMAKE_CURRENT_BINARY_DIR}/zw-libs2)
else()
  set(ZW-LIBS2_PATH ${DEFAULT_ZW-LIBS2_PATH})
  add_subdirectory(${ZW-LIBS2_PATH})
endif()

if(NOT BUILD_TESTING)
  set(SKIP_TESTING
      ON
      CACHE BOOL "Skip building tests for zw-libs2")
endif()

target_include_directories(
  zwave_s2
  PUBLIC include
  PRIVATE ${ZW-LIBS2_PATH}/include src)

install(TARGETS zwave_s2 LIBRARY DESTINATION lib)

if(NOT ZWAVE_BUILD_SYSTEM EQUAL 1)

endif()

target_link_libraries(
  zwave_s2
  PUBLIC zwave_s2_nonce_management unify
  PRIVATE s2_controller zwave_controller zwave_s0)

target_compile_definitions(zwave_s2 PRIVATE ZIPGW ZW_CONTROLLER)

if(BUILD_TESTING)
  target_add_mock(zwave_s2)
  add_subdirectory(test)

  add_mock(libs2_mock ${ZW-LIBS2_PATH}/include/S2.h
           ${ZW-LIBS2_PATH}/include/s2_inclusion.h
           ${ZW-LIBS2_PATH}/include/s2_protocol.h)
  target_interface_libraries(libs2_mock zwave_definitions s2crypto aes)
  target_compile_definitions(libs2_mock PUBLIC ZIPGW)
endif()
