if(NOT MQTT_CLIENT_LIBRARY)
  # Currently the default MQTT client-library is Mosquitto
  set(MQTT_CLIENT_LIBRARY Mosquitto)
endif(NOT MQTT_CLIENT_LIBRARY)

# Library

if(MQTT_CLIENT_LIBRARY STREQUAL Mosquitto)
  find_package(Mosquitto REQUIRED)
  add_library(uic_mqtt_wrapper SHARED src/mqtt_wrapper_mosquitto.cpp)

  target_link_libraries(uic_mqtt_wrapper ${MOSQUITTO_LIBRARIES} uic_log)
  target_include_directories(uic_mqtt_wrapper PRIVATE ${MOSQUITTO_INCLUDE_DIR})

  # elseif(MQTT_CLIENT_LIBRARY STREQUAL SomeOtherLibrary)
else()
  message(
    FATAL_ERROR "MQTT_CLIENT_LIBRARY '${MQTT_CLIENT_LIBRARY}' not implemented.")
endif()
add_library(
  uic_mqtt_client SHARED
  src/mqtt_client.cpp src/mqtt_client_fsm_connected.cpp
  src/mqtt_client_fsm_connecting.cpp src/mqtt_client_fsm_disconnected.cpp)
target_link_libraries(uic_mqtt_client uic_mqtt_wrapper)
target_include_directories(uic_mqtt_client PUBLIC include)
add_dependencies(uic_mqtt_client sl_status_strings)

add_mock(uic_mqtt_client_mock src/mqtt_client.h)
add_mock(uic_mqtt_wrapper_mock src/mqtt_wrapper.h)

add_library(uic_mqtt SHARED src/uic_mqtt.c)
target_include_directories(uic_mqtt PUBLIC include)
target_link_libraries(uic_mqtt uic_mqtt_client uic_main_fd)

target_add_mock(uic_mqtt)

add_subdirectory(test)
