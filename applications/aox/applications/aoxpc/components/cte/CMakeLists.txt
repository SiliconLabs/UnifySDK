# AoA Constant Tone Extension (CTE) handle component

add_library(cte
  src/aoa_cte.c
  src/cte_conn_less.c
  src/cte_conn.c
  src/cte_silabs.c
)

target_include_directories(cte
  PUBLIC
  include
  PRIVATE
  src
)

target_link_libraries(cte ncp db aoa_types aoa_utils antenna_array)

set_target_properties(cte PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "AOA_CTE"
)
