add_library(epc_application_monitoring_shared SHARED
            ../$<TARGET_PROPERTY:epc_application_monitoring,SOURCES>)
target_link_libraries(epc_application_monitoring_shared unify)
target_include_directories(epc_application_monitoring_shared PUBLIC ../include)

target_add_unittest(
  epc_application_monitoring_shared
  NAME
  epc_application_monitoring_test
  SOURCES
  epc_application_monitoring_test.c
  DEPENDS
  unify_application_monitoring_mock)
