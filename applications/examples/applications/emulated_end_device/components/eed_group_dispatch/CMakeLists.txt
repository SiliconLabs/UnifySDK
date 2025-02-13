add_library(eed_group_dispatch src/eed_group_dispatch.cpp)

target_include_directories(
  eed_group_dispatch
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  eed_group_dispatch
  PRIVATE unify
          eed_attribute_store
          )


