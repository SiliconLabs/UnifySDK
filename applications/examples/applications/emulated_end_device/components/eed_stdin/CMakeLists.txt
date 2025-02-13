find_package(Edit REQUIRED)
add_library(eed_stdin src/eed_stdin_command_handling.cpp src/eed_stdin_fixt.c)

target_include_directories(
  eed_stdin
  PUBLIC include
  PRIVATE ../uic_main/include src)

target_link_libraries(
  eed_stdin
  PUBLIC eed_attribute_store
         unify
)