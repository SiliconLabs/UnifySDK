# Library
add_library(uic_gbl_interface OBJECT src/uic_gbl_interface.cpp)
target_include_directories(
  uic_gbl_interface PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)

set_source_files_properties(src/uic_gbl_interface.cpp
                            PROPERTIES COMPILE_FLAGS -Wno-error=unused-result)
target_link_libraries(uic_gbl_interface uic_log uic_contiki_platform)

if(BUILD_TESTING)
  # Testing
  target_add_unittest(uic_gbl_interface SOURCES test/uic_gbl_interface_test.c
                      DEPENDS unify)

  file(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/zwave_ncp_serial_api_controller_BRD4201A.gbl
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

  # Mock
  target_add_mock(uic_gbl_interface)
endif()

install_headers(uic_gbl_interface uic libuic-dev)
