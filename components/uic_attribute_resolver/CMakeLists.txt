# Attribute resolver
add_library(uic_attribute_resolver OBJECT src/attribute_resolver.cpp
                                          src/attribute_resolver_rule.cpp)

target_include_directories(
  uic_attribute_resolver
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  PRIVATE src)
target_link_libraries(
  uic_attribute_resolver
  PUBLIC uic_attribute_store uic_datastore uic_multi_invoke uic_contiki
         uic_contiki_platform)

install_headers(uic_attribute_resolver uic libuic-dev)

if(BUILD_TESTING)
  # Attribute resolver mock
  add_mock(uic_attribute_resolver_mock include/attribute_resolver.h
           include/attribute_resolver_rule.h)
  target_include_directories(uic_attribute_resolver_mock PUBLIC include)
  target_interface_libraries(uic_attribute_resolver_mock uic_attribute_store
                             uic_contiki uic_log)

  add_subdirectory(test)
endif()
