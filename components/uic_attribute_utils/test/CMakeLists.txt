# Tests for the Attribute transitions
target_add_unittest(
  uic_attribute_utils
  NAME attribute_transitions_test
  SOURCES attribute_transitions_test.c
  DEPENDS
    uic_contiki_stub
    uic_attribute_store
)

# Tests for the Attribute transitions
target_add_unittest(
  uic_attribute_utils
  NAME attribute_timeouts_test
  SOURCES attribute_timeouts_test.c
  DEPENDS
    uic_contiki_stub
    uic_attribute_store
)

