add_library(uic_attribute_utils_shared SHARED $<TARGET_OBJECTS:uic_attribute_utils>)
target_link_libraries(uic_attribute_utils_shared unify)
target_include_directories(
  uic_attribute_utils_shared
  PUBLIC $<TARGET_PROPERTY:uic_attribute_utils,INCLUDE_DIRECTORIES>)

# Tests for the Attribute transitions
target_add_unittest(
  uic_attribute_utils_shared
  NAME
  attribute_transitions_test
  SOURCES
  attribute_transitions_test.c
  DEPENDS
  uic_contiki_stub
  )

# Tests for the Attribute transitions
target_add_unittest(
  uic_attribute_utils_shared
  NAME
  attribute_timeouts_test
  SOURCES
  attribute_timeouts_test.c
  DEPENDS
  uic_contiki_stub
)
