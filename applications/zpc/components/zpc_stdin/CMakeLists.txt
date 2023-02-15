find_package(Edit REQUIRED)
add_library(zpc_stdin src/zpc_stdin_command_handling.cpp src/zpc_stdin_fixt.c)

target_include_directories(
  zpc_stdin
  PUBLIC include
  PRIVATE ../uic_main/include src)

target_link_libraries(
  zpc_stdin
  PUBLIC zpc_utils
         zwave_tx_groups
         zwave_command_classes
         zwave_command_handler
         ucl_mqtt
         unify
  PRIVATE zwave_s2)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
