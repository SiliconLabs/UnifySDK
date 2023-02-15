# AoX Protocol Controller angle correction component
add_library(aoxpc_correction src/aoxpc_correction.c)

target_include_directories(aoxpc_correction PUBLIC include)

target_link_libraries(aoxpc_correction 
  PRIVATE aoa_angle aoa_util unify aoa_db aoxpc_unid)
