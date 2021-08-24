# Library
add_library(zwave_tx
  src/zwave_tx.cpp
  src/zwave_tx_callbacks.cpp
  src/zwave_tx_process.cpp
  src/zwave_tx_queue.cpp
  src/zwave_tx_state_logging.c
  src/zwave_tx_fixt.c
  src/zwave_tx_intercept_payload.cpp
  src/zwave_tx_follow_ups.cpp
  )

target_include_directories(
  zwave_tx
  PUBLIC include
  PRIVATE src)

target_link_libraries(zwave_tx
PUBLIC
  zwave_api
  uic_contiki_platform
  uic_contiki
  zwave_tx_groups
PRIVATE
  zwave_controller
)

target_add_mock(zwave_tx)

add_subdirectory(test)
