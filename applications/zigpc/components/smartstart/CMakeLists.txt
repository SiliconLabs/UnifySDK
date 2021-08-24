add_library(zigpc_smartstart src/zigpc_smartstart.cpp
                             src/zigpc_smartstart_dsk.cpp)

target_include_directories(
  zigpc_smartstart
  PRIVATE src
  PUBLIC include)
target_link_libraries(
  zigpc_smartstart PRIVATE zigpc_common zigpc_net_mgmt zigpc_node_mgmt uic_log
                           uic_smartstart_management)

add_subdirectory(test)

target_add_mock(zigpc_smartstart)
