add_library(uic_main_fd OBJECT src/uic_main_externals.c)
target_link_libraries(uic_main_fd PUBLIC uic_contiki uic_log)
target_include_directories(
  uic_main_fd
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  PRIVATE src/)

install_headers(uic_main_fd uic libuic-dev)

if(BUILD_TESTING)
  target_add_mock(uic_main_fd)
endif()
