add_library(
  uic_attribute_store OBJECT
  src/attribute_store.cpp
  src/attribute_store_callbacks.cpp
  src/attribute_store_helper.c
  src/attribute_store_node.cpp
  src/attribute_store_type_registration.cpp
  src/attribute_store_configuration.cpp
  src/attribute_store_validation.cpp
  src/attribute_store_process.c)

target_include_directories(
  uic_attribute_store
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  PRIVATE src)

target_link_libraries(
  uic_attribute_store PUBLIC uic_datastore uic_log uic_config uic_contiki
                             uic_contiki_platform)

install_headers(uic_attribute_store uic libuic-dev)

if(BUILD_TESTING)
  add_library(uic_attribute_store_test_helpers
              src/attribute_store_test_helpers.cpp)

  target_include_directories(uic_attribute_store_test_helpers PUBLIC ../src)

  target_link_libraries(uic_attribute_store_test_helpers
                        PUBLIC uic_datastore uic_attribute_store)
  # Mocks
  target_add_mock(uic_attribute_store)

  # Unit test subdirectory
  add_subdirectory(test)
endif()
