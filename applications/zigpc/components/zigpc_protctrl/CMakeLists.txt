set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

# Library
add_library(
  zigpc_protctrl
  platform/${COMPATIBLE_PLATFORM}/zigpc_protctrl_nwmgmt_write.cpp
  src/zigpc_protctrl_nwmgmt_write_callback.c src/zigpc_protctrl_process.c
  src/zigpc_protctrl_observers.c src/zigpc_protctrl_nwmgmt.c)

target_include_directories(
  zigpc_protctrl
  PRIVATE src ${Boost_INCLUDE_DIRS} platform/${COMPATIBLE_PLATFORM}
  PUBLIC include)
target_link_libraries(zigpc_protctrl PRIVATE zigpc_common zigpc_net_mgmt
                                             uic_contiki uic_mqtt uic_log)

target_add_mock(zigpc_protctrl)

add_subdirectory(test)
