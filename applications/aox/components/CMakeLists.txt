add_subdirectory(app_log_wrapper)
add_subdirectory(rtl_lib)
add_subdirectory(ucl_mqtt_helper)

# Components from GSDK

# ##############################################################################
# AoA angle calculation
# ##############################################################################
add_library(aoa_angle
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_angle/aoa_angle.c
)
target_include_directories(aoa_angle
  PUBLIC ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_angle
  PRIVATE ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_angle/config
)
target_link_libraries(aoa_angle
  rtl_lib aoa_util antenna_array app_log_wrapper
)
set_target_properties(aoa_angle PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "AOA_ANGLE"
)

# ##############################################################################
# Configuration parser
# ##############################################################################
set_source_files_properties(
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_parse/aoa_parse.c PROPERTIES COMPILE_FLAGS -Wno-error=unused-result
)
add_library(aoa_parse
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_parse/aoa_parse.c
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_parse/aoa_parse_enum.c
)
target_include_directories(aoa_parse
  PUBLIC ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_parse
)

target_link_libraries(aoa_parse
  PUBLIC aoa_util aoa_cjson
  PRIVATE rtl_lib)

# ##############################################################################
# AoA utilities and type definitions
# ##############################################################################
add_library(aoa_util
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_util/aoa_util.c
)
target_include_directories(aoa_util
  PUBLIC ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_util
)

target_link_libraries(aoa_util aoa_cjson)

target_compile_options(aoa_util
  PRIVATE
    $<$<C_COMPILER_ID:GNU>:-Wno-stringop-truncation>
)

# ##############################################################################
# Antenna Array
# ##############################################################################
add_library(antenna_array
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/antenna_array/antenna_array.c
)

target_include_directories(antenna_array
  PUBLIC ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/antenna_array
)

# ##############################################################################
# cJSON 3rd party JSON parser
# ##############################################################################
add_library(aoa_cjson
  ${GeckoSDK_ROOT_DIR}/util/third_party/cjson/cJSON.c
)

target_include_directories(aoa_cjson
  PUBLIC ${GeckoSDK_ROOT_DIR}/util/third_party/cjson
)

target_link_libraries(aoa_cjson m)

target_compile_options(aoa_cjson
  PRIVATE -Wno-misleading-indentation
)

set_target_properties(aoa_cjson PROPERTIES OUTPUT_NAME "cjson")
