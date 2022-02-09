# Library
add_library(
  zwave_tx
  src/zwave_tx.cpp
  src/zwave_tx_callbacks.cpp
  src/zwave_tx_fixt.c
  src/zwave_tx_follow_ups.cpp
  src/zwave_tx_process.cpp
  src/zwave_tx_queue.cpp
  src/zwave_tx_state_logging.c)

target_include_directories(
  zwave_tx
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  zwave_tx
  PUBLIC uic_contiki zwave_tx_groups zwave_definitions zwave_api
  PRIVATE zwave_controller)
install(TARGETS zwave_tx LIBRARY DESTINATION lib)

target_add_mock(zwave_tx)

add_subdirectory(test)
