add_library(positioning_application_monitoring_shared SHARED
            ../$<TARGET_PROPERTY:positioning_application_monitoring,SOURCES>)
target_link_libraries(positioning_application_monitoring_shared unify)
target_include_directories(
  positioning_application_monitoring_shared
  PUBLIC
    $<TARGET_PROPERTY:positioning_application_monitoring,INCLUDE_DIRECTORIES>)
target_add_unittest(
  positioning_application_monitoring_shared
  NAME
  positioning_application_monitoring_test
  SOURCES
  positioning_application_monitoring_test.c
  DEPENDS
  unify_application_monitoring_mock)
