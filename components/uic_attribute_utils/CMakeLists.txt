# Attribute transitions
add_library(uic_attribute_utils SHARED
    src/attribute_transitions.cpp
    src/attribute_timeouts.cpp
)

target_include_directories(uic_attribute_utils
  PUBLIC include
  PRIVATE src
)
target_link_libraries(uic_attribute_utils PUBLIC
  uic_attribute_store
  uic_contiki
  uic_contiki_platform
)

# Unit tests
add_subdirectory(test)

# Mocks
target_add_mock(uic_attribute_utils)