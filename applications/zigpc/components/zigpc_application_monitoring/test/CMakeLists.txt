target_include_directories(
  zigpc_application_monitoring_shared
  PUBLIC $<TARGET_PROPERTY:zigpc_application_monitoring,INCLUDE_DIRECTORIES>)
target_add_unittest(
  zigpc_application_monitoring_shared
  NAME
  zigpc_application_monitoring_test
  SOURCES
  zigpc_application_monitoring_test.c
  DEPENDS
  unify_application_monitoring_mock)
