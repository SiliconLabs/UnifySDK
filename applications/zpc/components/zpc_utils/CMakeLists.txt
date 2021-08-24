add_library(zpc_utils
  src/zpc_converters.c
  src/zwave_unid.c
  src/zpc_hex_to_int.c
  src/zwave_utils.c
  src/zpc_crc16.c
)

target_include_directories(
  zpc_utils
  PUBLIC include
  # FIXME: Find another way of getting zwave_dsk_t for the zpc_converters
  PRIVATE ${ZPC_SOURCE_DIR}/components/zwave_controller/include)
target_link_libraries(zpc_utils uic_log zpc_attribute_store zwave_controller)

# Mocks
add_mock(zpc_utils_mock include/zwave_utils.h)
add_mock(zwave_unid_mock include/zwave_unid.h)

add_subdirectory(test)
