# Configuration parser component

add_library(aoa_parse src/aoa_parse.c)

target_include_directories(aoa_parse PUBLIC include)

target_link_libraries(aoa_parse
  PUBLIC aoa_types aoa_utils cjson
  PRIVATE rtl_lib)
