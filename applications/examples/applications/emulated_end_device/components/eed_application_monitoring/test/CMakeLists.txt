add_library(eed_application_monitoring_shared SHARED
            ../$<TARGET_PROPERTY:eed_application_monitoring,SOURCES>)
target_link_libraries(eed_application_monitoring_shared unify)
target_include_directories(eed_application_monitoring_shared PUBLIC ../include)

target_add_unittest(
  eed_application_monitoring_shared
  NAME
  eed_application_monitoring_test
  SOURCES
  eed_application_monitoring_test.c
  DEPENDS
  unify_application_monitoring_mock)
