# Library
add_library(zwave_command_handler src/zwave_command_handler.cpp
                                  src/zwave_command_handler_callbacks.cpp
                                  zwave_command_type.cpp
                                  )

set(CC_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/../zwave_command_classes/assets")
add_custom_command( OUTPUT zwave_command_type.cpp
DEPENDS
  ${CC_ASSETS}/ZWave_custom_cmd_classes.xml
  ${CC_ASSETS}/supported.csv
  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/extract_get.py
COMMAND
  ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/extract_get.py
  ${CC_ASSETS}/ZWave_custom_cmd_classes.xml
  ${CC_ASSETS}/supported.csv > zwave_command_type.cpp
)

target_include_directories(
  zwave_command_handler
  PUBLIC include
  PRIVATE src)
target_link_libraries(
  zwave_command_handler
  PUBLIC zwave_controller zwave_security_validation)
install(TARGETS zwave_command_handler LIBRARY DESTINATION lib)

if(BUILD_TESTING)
  # Mocks
  target_add_mock(zwave_command_handler)

  # Unit test subdirectory
  add_subdirectory(test)
endif()
