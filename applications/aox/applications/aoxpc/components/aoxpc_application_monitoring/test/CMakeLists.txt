add_library(aox_application_monitoring_test SHARED
            ../$<TARGET_PROPERTY:aoxpc_application_monitoring,SOURCES>)
target_link_libraries(aox_application_monitoring_test unify)
target_include_directories(
  aox_application_monitoring_test
  PUBLIC $<TARGET_PROPERTY:aoxpc_application_monitoring,INCLUDE_DIRECTORIES>)
target_add_unittest(
  aox_application_monitoring_test
  NAME
  aoxpc_application_monitoring_test
  SOURCES
  aoxpc_application_monitoring_test.c
  DEPENDS
  unify_application_monitoring_mock)
