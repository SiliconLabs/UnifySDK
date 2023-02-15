# Library

# Evil 😼 hack for disabling the -pipe option to gcc, wich caused GCC to produce
# bad code for the assembler. This has been observed with
# arm-linux-gnueabihf-g++ (Debian 8.3.0-2) 8.3.0 Its likely caused by the fact
# that the dotdot_mqtt.cpp file becomes quiete large. Setting CMAKE_CXX_FLAGS
# should only affect this directory and is children.
if(CMAKE_CXX_FLAGS MATCHES "-pipe")
  string(REPLACE "-pipe" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

# Find nlohmann_json, unfortunately the find_package doesn't work for this when
# cross compiling, thus we use the simplified method find_path instead
find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

run_zap(zap/gen-templates.json)

add_library(
  uic_dotdot_mqtt OBJECT
  ${ZAP_CURRENT_OUTPUT_DIR}/src/dotdot_mqtt.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/dotdot_mqtt_attributes.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/dotdot_mqtt_command_helpers.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/dotdot_mqtt_send_commands.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/dotdot_mqtt_group_commands.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/dotdot_mqtt_generated_commands.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/dotdot_mqtt_supported_generated_commands.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/dotdot_mqtt_helpers.cpp
  src/dotdot_mqtt_parsing_helpers.cpp
  src/dotdot_bitmap.cpp
  src/json_helpers.cpp)

target_include_directories(
  uic_dotdot_mqtt
  PRIVATE src ${ZAP_CURRENT_OUTPUT_DIR}/src ${nlohmann_json_include}
  PUBLIC include $<BUILD_INTERFACE:${ZAP_CURRENT_OUTPUT_DIR}/include>
         $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>)

target_link_libraries(uic_dotdot_mqtt PUBLIC uic_definitions uic_dotdot
                                             uic_mqtt uic_log)

if(BUILD_TESTING)
  add_mock(
    uic_dotdot_mqtt_mock
    ${ZAP_CURRENT_OUTPUT_DIR}/include/dotdot_mqtt.h
    ${ZAP_CURRENT_OUTPUT_DIR}/include/dotdot_mqtt_group_commands.h
    ${ZAP_CURRENT_OUTPUT_DIR}/include/dotdot_mqtt_supported_generated_commands.h
    ${ZAP_CURRENT_OUTPUT_DIR}/include/dotdot_mqtt_generated_commands.h)

  target_interface_libraries(uic_dotdot_mqtt_mock uic_definitions uic_dotdot
                             uic_mqtt uic_log)
  target_include_directories(
    uic_dotdot_mqtt_mock
    PRIVATE src ${ZAP_CURRENT_OUTPUT_DIR}/src ${nlohmann_json_include}
    PUBLIC ${ZAP_CURRENT_OUTPUT_DIR}/include ${Boost_INCLUDE_DIRS})

  add_subdirectory(test)
endif()

install_headers(uic_dotdot_mqtt uic libuic-dev)
