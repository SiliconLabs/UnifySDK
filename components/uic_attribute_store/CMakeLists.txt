# Library
add_library(
  uic_attribute_store SHARED
  src/attribute_store.cpp
  src/attribute_store_callbacks.cpp
  src/attribute_store_helper.c
  src/attribute_store_node.cpp
  src/attribute_store_type_registration.cpp
  src/attribute_store_configuration.cpp
  src/attribute_store_validation.cpp)

target_include_directories(
  uic_attribute_store
  PUBLIC include
  PRIVATE src)

target_link_libraries(uic_attribute_store PUBLIC uic_datastore)

add_library(uic_attribute_store_test_helpers
            src/attribute_store_test_helpers.cpp)

target_include_directories(uic_attribute_store_test_helpers PUBLIC ../src)

target_link_libraries(uic_attribute_store_test_helpers
                      PUBLIC uic_datastore uic_attribute_store)

# For using static library:
disable_target_check(uic_attribute_store_test_helpers)

# Mocks
target_add_mock(uic_attribute_store)

# Unit test subdirectory
add_subdirectory(test)
