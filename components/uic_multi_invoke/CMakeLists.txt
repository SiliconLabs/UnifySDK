add_library(uic_multi_invoke INTERFACE)
target_include_directories(
  uic_multi_invoke
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)

install_headers(uic_multi_invoke uic libuic-dev)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
