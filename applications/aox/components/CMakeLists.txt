add_subdirectory(antenna_array)
add_subdirectory(aoa_angle)
add_subdirectory(aoa_parse)
add_subdirectory(aoa_types)
add_subdirectory(aoa_utils)
add_subdirectory(ucl_mqtt_helper)

# ##############################################################################
# cJSON 3rd party JSON parser
# ##############################################################################
add_library(cjson
  ${GeckoSDK_ROOT_DIR}/util/third_party/cjson/cJSON.c
)

target_include_directories(cjson
  PUBLIC ${GeckoSDK_ROOT_DIR}/util/third_party/cjson
)

target_link_libraries(cjson m)

target_compile_options(cjson
  PRIVATE -Wno-misleading-indentation
)

# ##############################################################################
# Real-Time Locationing library
# ##############################################################################
add_library(rtl_lib STATIC IMPORTED GLOBAL)

target_link_libraries(rtl_lib INTERFACE stdc++)
target_link_options(rtl_lib INTERFACE $<$<C_COMPILER_ID:GNU>:-no-pie>)

# Find library for current platform
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME)
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} SYSTEM_PROCESSOR)
# TODO: remove this workaround once CMAKE_SYSTEM_PROCESSOR is fixed for RPI.
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armhf")
  set(SYSTEM_PROCESSOR "armv7l")
endif()

set_target_properties(rtl_lib PROPERTIES
  IMPORTED_LOCATION ${GeckoSDK_ROOT_DIR}/util/silicon_labs/aox/lib/gcc/release/libaox_static_${SYSTEM_NAME}_${SYSTEM_PROCESSOR}.a
  INTERFACE_INCLUDE_DIRECTORIES ${GeckoSDK_ROOT_DIR}/util/silicon_labs/aox/inc
  INTERFACE_COMPILE_DEFINITIONS "RTL_LIB"
)
