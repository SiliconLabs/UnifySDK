add_library(unify_validator OBJECT src/unify_validator.cpp)
target_include_directories(
  unify_validator PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)
install_headers(unify_validator uic libuic-dev)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
