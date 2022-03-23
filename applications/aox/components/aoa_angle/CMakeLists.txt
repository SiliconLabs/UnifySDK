# AoA angle calculation component

add_library(aoa_angle src/aoa_angle.c)

target_include_directories(aoa_angle PUBLIC include PRIVATE src)

target_link_libraries(aoa_angle rtl_lib aoa_types aoa_utils antenna_array)

set_target_properties(aoa_angle PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "AOA_ANGLE"
)
