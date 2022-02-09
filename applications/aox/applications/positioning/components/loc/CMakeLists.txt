# Location calculation engine

add_library(loc src/aoa_loc.c)
target_include_directories(loc PUBLIC include)
target_link_libraries(loc rtl_lib aoa_types aoa_utils)
