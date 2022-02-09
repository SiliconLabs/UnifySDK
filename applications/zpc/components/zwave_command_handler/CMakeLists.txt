# Library
add_library(zwave_command_handler src/zwave_command_handler.cpp
                                  src/zwave_command_handler_callbacks.cpp)
target_include_directories(
  zwave_command_handler
  PUBLIC include
  PRIVATE src)
target_link_libraries(
  zwave_command_handler
  PUBLIC zwave_controller zwave_security_validation zwave_network_management
         zwave_rx zwave_definitions)
install(TARGETS zwave_command_handler LIBRARY DESTINATION lib)

# Mocks
target_add_mock(zwave_command_handler)

# Unit test subdirectory
add_subdirectory(test)
