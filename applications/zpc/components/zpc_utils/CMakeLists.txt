add_library(zpc_utils src/zpc_converters.c src/zpc_hex_to_int.c
                      src/zwave_unid.c)

target_include_directories(
  zpc_utils
  PUBLIC include
  # FIXME: Find another way of getting zwave_dsk_t for the zpc_converters
  PRIVATE ${ZPC_SOURCE_DIR}/components/zwave_controller/include)

target_link_libraries(
  zpc_utils
  PUBLIC zwave_definitions
  INTERFACE zwave_controller
  PRIVATE uic_log zwave_controller)
install(TARGETS zpc_utils LIBRARY DESTINATION lib)

# Mocks
add_mock(zpc_utils_mock include/zpc_converters.h include/zwave_unid.h
         include/zpc_hex_to_int.h)
target_link_libraries(zpc_utils_mock PUBLIC zwave_definitions
                                            zwave_controller_mock)
add_mock(zwave_unid_mock include/zwave_unid.h)
target_link_libraries(zwave_unid_mock PUBLIC zwave_definitions)

add_subdirectory(test)
