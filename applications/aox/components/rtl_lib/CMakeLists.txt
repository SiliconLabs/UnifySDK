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
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
  set(SYSTEM_PROCESSOR "aarch64")
endif()

set_target_properties(rtl_lib PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/gcc/release/libaox_static_${SYSTEM_NAME}_${SYSTEM_PROCESSOR}.a
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/inc
  INTERFACE_COMPILE_DEFINITIONS "RTL_LIB"
)
