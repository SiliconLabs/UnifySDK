if(NOT MQTT_CLIENT_LIBRARY)
  # Currently the default MQTT client-library is Mosquitto
  set(MQTT_CLIENT_LIBRARY Mosquitto)
endif(NOT MQTT_CLIENT_LIBRARY)

# Library
if(MQTT_CLIENT_LIBRARY STREQUAL Mosquitto)
  find_package(Mosquitto REQUIRED)
  set(MQTT_LIBRARY ${MOSQUITTO_LIBRARIES})
  set(MQTT_LIBRARY_INCLUDE_DIR ${MOSQUITTO_INCLUDE_DIR})
  set(MQTT_LIBRARY_WRAPPER_SRC src/mqtt_wrapper_mosquitto.cpp)

  # elseif(MQTT_CLIENT_LIBRARY STREQUAL SomeOtherLibrary)
else()
  message(
    FATAL_ERROR "MQTT_CLIENT_LIBRARY '${MQTT_CLIENT_LIBRARY}' not implemented.")
endif()

add_library(
  uic_mqtt OBJECT
  src/mqtt_client.cpp
  src/mqtt_client_fsm_connected.cpp
  src/mqtt_client_fsm_connecting.cpp
  src/mqtt_client_fsm_disconnected.cpp
  src/mqtt_client_id.cpp
  ${MQTT_LIBRARY_WRAPPER_SRC}
  src/uic_mqtt.c)

target_include_directories(
  uic_mqtt
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
  PRIVATE src ${MQTT_LIBRARY_INCLUDE_DIR})

target_link_libraries(uic_mqtt PUBLIC ${MQTT_LIBRARY} uic_contiki uic_main_fd
                                      uic_config uic_log uic_datastore)
add_dependencies(uic_mqtt sl_status_strings)

install_headers(uic_mqtt uic libuic-dev)

if(BUILD_TESTING)
  find_package(Mosquitto REQUIRED)
  add_library(uic_mqtt_stub $<TARGET_OBJECTS:uic_mqtt_mock>
                            test/mqtt_test_helper.cpp)
  target_link_libraries(
    uic_mqtt_stub PUBLIC ${MOSQUITTO_LIBRARIES} uic_mqtt_mock
                         $<TARGET_PROPERTY:uic_mqtt_mock,LINK_LIBRARIES> unify)
  target_include_directories(
    uic_mqtt_stub
    PRIVATE ${MOSQUITTO_INCLUDE_DIR}
    PUBLIC test $<TARGET_PROPERTY:uic_mqtt_mock,include_directories>)
  target_add_mock(uic_mqtt)
  add_mock(uic_mqtt_client_mock src/mqtt_client.h src/mqtt_client_id.h)
  add_mock(uic_mqtt_wrapper_mock src/mqtt_wrapper.h)
  add_subdirectory(test)
endif()
