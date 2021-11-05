# Source
set(zigpc_net_mgmt_SRC
    src/zigpc_net_mgmt_fsm.c src/zigpc_net_mgmt_fsm_log.c
    src/zigpc_net_mgmt_process.c src/zigpc_net_mgmt_gateway.c
    src/zigpc_net_mgmt.c src/zigpc_net_mgmt_notify.c)

# Library
add_library(zigpc_net_mgmt ${zigpc_net_mgmt_SRC})
target_include_directories(
  zigpc_net_mgmt
  PUBLIC include
  PRIVATE src)
target_link_libraries(
  zigpc_net_mgmt PRIVATE zigpc_common zigpc_gateway zigpc_datastore uic_log
                         uic_contiki)

target_add_mock(zigpc_net_mgmt)

add_subdirectory(test)
