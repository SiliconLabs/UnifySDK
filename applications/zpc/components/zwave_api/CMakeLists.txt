add_library(
  zwave_api
  src/zwapi_connection.c
  src/zwapi_init.c
  src/zwapi_protocol_basis.c
  src/zwapi_protocol_controller.c
  src/zwapi_protocol_mem.c
  src/zwapi_protocol_nvm.c
  src/zwapi_protocol_rx_dispatch.c
  src/zwapi_protocol_transport.c
  src/zwapi_session.c
  src/zwapi_utils.c
  platform/${COMPATIBLE_PLATFORM}/zwapi_serial.c
  platform/${COMPATIBLE_PLATFORM}/zwapi_timestamp.c
)

target_link_libraries(zwave_api PUBLIC zwave_definitions PRIVATE unify)
target_include_directories(
  zwave_api
  PRIVATE src platform/${COMPATIBLE_PLATFORM}
  PUBLIC include
  # Needed for Z-Wave controller endian, we do not want to pull the whole Z-Wave Controller library here.
  $<TARGET_PROPERTY:zwave_controller,INTERFACE_INCLUDE_DIRECTORIES>
  )
install(TARGETS zwave_api LIBRARY DESTINATION lib)

if(BUILD_TESTING)
  target_add_mock(zwave_api)
  target_interface_libraries(zwave_api_mock zwave_definitions unify)

  # Mock within the component
  add_mock(zwapi_internal_mock src/zwapi_connection.h src/zwapi_session.h)
  target_interface_libraries(zwapi_internal_mock zwave_api)

  add_library(zwapi_protocol_mem_mock
              ${CMAKE_CURRENT_LIST_DIR}/mocks/zwapi_protocol_mem_mock.c)
  target_link_libraries(zwapi_protocol_mem_mock PRIVATE cmock zwave_definitions unify)
  add_subdirectory(test)
endif()
