find_package(Edit REQUIRED)
add_library(
  uic_stdin OBJECT
  platform/posix/uic_stdin_process.c
  src/uic_stdin_command_handler.cpp
  src/unify_stdin_attribute_resolver_command_handler.cpp
  src/unify_stdin_attribute_poll_command_handler.cpp
  src/unify_stdin_attribute_store_command_handler.cpp)

target_include_directories(
  uic_stdin
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  PRIVATE ../uic_main/include src)

target_link_libraries(
  uic_stdin
  PUBLIC uic_main_fd
  PRIVATE unify_attribute_poll edit pthread uic_attribute_resolver
          uic_attribute_store uic_log)

install_headers(uic_stdin uic libuic-dev)

if(BUILD_TESTING)
  # Unit tests
  add_subdirectory(test)
endif()
