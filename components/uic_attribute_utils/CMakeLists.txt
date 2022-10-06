# Attribute transitions
add_library(uic_attribute_utils
    src/attribute_transitions.cpp
    src/attribute_timeouts.cpp
)

target_include_directories(uic_attribute_utils
  PUBLIC include
  PRIVATE src
)

target_link_libraries(uic_attribute_utils
  PUBLIC
    uic_attribute_store
  PRIVATE
    uic_contiki
    uic_contiki_platform
)

install_headers(uic_attribute_utils "uic" "uic-dev")
# Unit tests
add_subdirectory(test)

# Mocks
target_add_mock(uic_attribute_utils)

# Test helper using the contiki stub
add_library(uic_attribute_utils_test_helper SHARED
    src/attribute_transitions.cpp
    src/attribute_timeouts.cpp
)
target_include_directories(uic_attribute_utils_test_helper
  PUBLIC include
  PRIVATE src
)
target_link_libraries(uic_attribute_utils_test_helper
  PUBLIC
    uic_attribute_store
  PRIVATE
    uic_contiki_stub
)
