# Library

# Evil 😼 hack for disabling the -pipe option to gcc, wich caused GCC to
# produce bad code for the assembler. This has been observed with
# arm-linux-gnueabihf-g++ (Debian 8.3.0-2) 8.3.0
# Its likely caused by the fact that the dotdot_mqtt.cpp file becomes
# quiete large.
# Setting CMAKE_CXX_FLAGS should only affect this directory
# and is children.
string(REPLACE "-pipe" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

find_program(ZAP_EXECUTABLE "zap" REQUIRED)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

RUN_ZAP(zap/gen-templates.json)


add_library(uic_dotdot_mqtt SHARED ${CMAKE_CURRENT_BINARY_DIR}/src/dotdot_mqtt.cpp src/dotdot_bitmap.cpp)

target_include_directories(uic_dotdot_mqtt
  PRIVATE
    src
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${Boost_INCLUDE_DIRS})
target_link_libraries(uic_dotdot_mqtt uic_definitions uic_dotdot_types uic_mqtt)

add_mock(uic_dotdot_mqtt_mock ${CMAKE_CURRENT_BINARY_DIR}/include/dotdot_mqtt.h)
target_interface_libraries(uic_dotdot_mqtt_mock uic_definitions uic_dotdot_types uic_mqtt)
target_include_directories(uic_dotdot_mqtt_mock
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)

add_subdirectory(test)
