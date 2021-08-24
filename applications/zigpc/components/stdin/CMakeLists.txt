find_package(Edit REQUIRED)
add_library(zigpc_stdin src/zigpc_stdin_commands.cpp)

target_include_directories(
  zigpc_stdin
  PUBLIC include
  PRIVATE src)

target_link_libraries(zigpc_stdin PRIVATE zigpc_gateway uic_stdin)

add_subdirectory(test)
