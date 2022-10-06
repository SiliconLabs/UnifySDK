find_package(Edit REQUIRED)
add_library(uic_stdin platform/posix/uic_stdin_process.c
                             src/uic_stdin_command_handler.cpp
                             src/unify_stdin_attribute_store_command_handler.cpp)

target_include_directories(
  uic_stdin
  PUBLIC include
  PRIVATE ../uic_main/include src)

target_link_libraries(
  uic_stdin
  PUBLIC uic_main_fd
  PRIVATE edit pthread uic_attribute_store)

install_headers(uic_stdin "uic" "uic-dev")

# Unit tests
add_subdirectory(test)