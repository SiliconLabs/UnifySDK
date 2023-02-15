# Library
add_library(uic_definitions INTERFACE)
target_include_directories(
  uic_definitions
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)
install_headers(uic_definitions uic libuic-dev)
