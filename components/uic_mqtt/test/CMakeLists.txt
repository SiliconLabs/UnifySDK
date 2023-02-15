# MQTT Library
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

add_library(uic_mqtt_shared SHARED
            "../$<JOIN:$<TARGET_PROPERTY:uic_mqtt,SOURCES>,;../>")
target_link_libraries(uic_mqtt_shared unify ${MQTT_LIBRARY})

target_add_unittest(
  uic_mqtt_shared
  NAME
  uic_mqtt_client_test
  SOURCES
  mqtt_client_test.cpp
  DEPENDS
  uic_mqtt_wrapper_mock
  unify_application_monitoring_mock
  EXCLUDE
  mqtt_wrapper_mosquitto.cpp)
target_include_directories(uic_mqtt_client_test PRIVATE ../src)

target_add_unittest(
  uic_mqtt_shared
  NAME
  uic_mqtt_test
  SOURCES
  uic_mqtt_test.c
  DEPENDS
  uic_contiki_stub
  uic_config_mock
  uic_main_fd_mock
  uic_mqtt_client_mock
  EXCLUDE
  mqtt_client_id.cpp
  mqtt_client.cpp)
target_include_directories(uic_mqtt_test PRIVATE ../src)

# MQTT Wrapper test
target_add_unittest(uic_mqtt_shared NAME mqtt_wrapper_mosquitto_test SOURCES
                    mqtt_wrapper_mosquitto_test.c)
target_include_directories(mqtt_wrapper_mosquitto_test PRIVATE ../src)

# MQTT Client ID test
target_add_unittest(
  uic_mqtt_shared
  NAME
  mqtt_client_id_test
  SOURCES
  mqtt_client_id_test.c
  DEPENDS
  uic_config_mock
  uic_datastore_mock)
target_include_directories(mqtt_client_id_test PRIVATE ../src)
