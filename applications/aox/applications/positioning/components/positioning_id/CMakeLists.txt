add_library(positioning_id
  src/positioning_id.c)

target_include_directories(
  positioning_id
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  positioning_id
  PUBLIC aoa_utils
  PRIVATE uic_log)
