add_library(uic_stdin_shared SHARED $<TARGET_OBJECTS:uic_stdin>)
target_link_libraries(uic_stdin_shared unify edit pthread)
target_include_directories(
  uic_stdin_shared PUBLIC $<TARGET_PROPERTY:uic_stdin,INCLUDE_DIRECTORIES>)

target_add_unittest(
  uic_stdin_shared
  NAME
  uic_stdin_test
  SOURCES
  uic_stdin_test.c
  DEPENDS
  unify)

# Attribute Store commands tests
target_add_unittest(
  uic_stdin_shared
  NAME
  unify_stdin_attribute_store_commands_test
  SOURCES
  unify_stdin_attribute_store_commands_test.c
  DEPENDS
  uic_attribute_store_mock
  DEPENDS
  unify)

# Test command handlers with the real attribute store
target_add_unittest(
  uic_stdin_shared
  NAME
  unify_stdin_attribute_store_command_handlers_test
  SOURCES
  unify_stdin_attribute_store_command_handlers_test.c
  DEPENDS
  unify)

# Attribute Resolver commands tests
target_add_unittest(
  uic_stdin_shared
  NAME
  unify_stdin_attribute_resolver_commands_test
  SOURCES
  unify_stdin_attribute_resolver_commands_test.c
  DEPENDS
  uic_attribute_resolver_mock
  unify)

# Attribute poll commands tests
target_add_unittest(uic_stdin_shared
NAME unify_stdin_attribute_poll_commands_test
SOURCES unify_stdin_attribute_poll_commands_test.c
DEPENDS unify_attribute_poll_mock unify)