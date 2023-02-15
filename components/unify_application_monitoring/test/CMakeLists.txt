add_library(unify_application_monitoring_shared SHARED
            $<TARGET_OBJECTS:unify_application_monitoring>)

target_link_libraries(unify_application_monitoring_shared unify)

target_include_directories(
  unify_application_monitoring_shared
  PUBLIC $<TARGET_PROPERTY:unify_application_monitoring,INCLUDE_DIRECTORIES>)

target_add_unittest(
  unify_application_monitoring_shared
  NAME
  unify_application_monitoring_test
  SOURCES
  unify_application_monitoring_test.c
  DEPENDS
  uic_mqtt_mock
  uic_dotdot_mqtt_mock
  EXCLUDE
  uic_mqtt
  uic_dotdot_mqtt)
