# AoA Constant Tone Extension (CTE) configuration parser component

add_library(cte_parse
  src/aoa_cte_parse.c
)
target_include_directories(cte_parse PUBLIC include)
target_link_libraries(cte_parse PUBLIC cte aoa_types PRIVATE aoa_parse cjson aoa_utils)