set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

add_library(
  zigpc_datastore src/zigpc_datastore.cpp src/zigpc_datastore_accessor.cpp
                  src/zigpc_datastore_util.cpp)

target_include_directories(
  zigpc_datastore
  PUBLIC include
  PRIVATE src)

target_link_libraries(zigpc_datastore PRIVATE zigpc_common unify
                                              pthread)

if(BUILD_TESTING)
  add_subdirectory(test)

  target_add_mock(zigpc_datastore)
endif()
