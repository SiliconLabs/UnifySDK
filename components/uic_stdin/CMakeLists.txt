find_package(Edit REQUIRED)
add_library(uic_stdin SHARED platform/posix/uic_stdin_process.c
                             src/uic_stdin_command_handler.cpp)

target_include_directories(
  uic_stdin
  PUBLIC include
  PRIVATE ../uic_main/include src)

target_link_libraries(
  uic_stdin
  PUBLIC uic_main_fd
  PRIVATE edit pthread)

target_add_unittest(uic_stdin SOURCES test/uic_stdin_test.c)
