# AoA utilities component

add_library(aoa_utils
  src/aoa_util.c
)

target_include_directories(aoa_utils PUBLIC include)

target_link_libraries(aoa_utils aoa_types cjson)

target_compile_options(aoa_utils
  PRIVATE
    $<$<C_COMPILER_ID:GNU>:-Wno-stringop-truncation>
)
