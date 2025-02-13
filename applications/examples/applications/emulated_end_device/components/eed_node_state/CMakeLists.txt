add_library(eed_node_state src/eed_node_state.c)

target_include_directories(
  eed_node_state
  PUBLIC include)

target_link_libraries(
  eed_node_state
  PUBLIC eed_attribute_store
         unify
)