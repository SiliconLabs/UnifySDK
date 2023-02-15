# Attribute transitions
add_library(uic_attribute_utils OBJECT src/attribute_transitions.cpp
                                       src/attribute_timeouts.cpp)

target_include_directories(
  uic_attribute_utils
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  PRIVATE src)

target_link_libraries(
  uic_attribute_utils
  PUBLIC uic_attribute_store
  PRIVATE uic_contiki uic_contiki_platform)

install_headers(uic_attribute_utils uic libuic-dev)

if(BUILD_TESTING)
  # Unit tests
  add_subdirectory(test)

  # Mocks
  target_add_mock(uic_attribute_utils)

  # Test helper using the contiki stub
  add_library(uic_attribute_utils_test_helper src/attribute_timeouts.cpp
                                              src/attribute_transitions.cpp)
  target_include_directories(
    uic_attribute_utils_test_helper
    PUBLIC include
    PRIVATE src)
  target_link_libraries(
    uic_attribute_utils_test_helper
    PUBLIC uic_attribute_store
    PRIVATE uic_contiki_stub)
endif()
