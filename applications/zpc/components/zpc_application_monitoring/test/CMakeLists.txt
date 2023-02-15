add_library(zpc_application_monitoring_shared SHARED
            ../$<TARGET_PROPERTY:zpc_application_monitoring,SOURCES>)
target_link_libraries(zpc_application_monitoring_shared unify)
target_include_directories(
  zpc_application_monitoring_shared
  PUBLIC $<TARGET_PROPERTY:zpc_application_monitoring,INCLUDE_DIRECTORIES>)

target_add_unittest(
  zpc_application_monitoring_shared
  NAME
  zpc_application_monitoring_test
  SOURCES
  zpc_application_monitoring_test.c
  DEPENDS
  unify_application_monitoring_mock)
