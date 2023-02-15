add_library(zpc_utils src/zpc_converters.c src/zpc_hex_to_int.c
                      src/zwave_unid.c)

target_include_directories(zpc_utils PUBLIC include)

target_link_libraries(
  zpc_utils
  PUBLIC zwave_definitions
  PRIVATE unify)

install(TARGETS zpc_utils LIBRARY DESTINATION lib)

if(BUILD_TESTING)
  # Mocks
  add_mock(zpc_utils_mock include/zpc_converters.h include/zwave_unid.h
           include/zpc_hex_to_int.h)
  target_link_libraries(zpc_utils_mock PUBLIC zwave_definitions
                                              zwave_controller_mock)
  add_mock(zwave_unid_mock include/zwave_unid.h)
  target_link_libraries(zwave_unid_mock PUBLIC zwave_definitions unify)

  add_subdirectory(test)
endif()
