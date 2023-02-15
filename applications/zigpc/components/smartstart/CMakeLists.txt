add_library(zigpc_smartstart src/zigpc_smartstart.cpp
                             src/zigpc_smartstart_dsk.cpp)

target_include_directories(
  zigpc_smartstart
  PRIVATE src
  PUBLIC include)
target_link_libraries(
  zigpc_smartstart PRIVATE zigpc_common zigpc_ucl zigpc_net_mgmt unify
                           zigpc_gateway )

if(BUILD_TESTING)
  add_subdirectory(test)

  target_add_mock(zigpc_smartstart)
endif()
