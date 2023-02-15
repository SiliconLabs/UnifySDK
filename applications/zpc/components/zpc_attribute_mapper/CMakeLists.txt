# ZPC Attribute Mapper library
add_library(zpc_attribute_mapper src/zpc_attribute_mapper.c)
target_link_libraries(zpc_attribute_mapper PRIVATE unify zpc_attribute_store)
target_include_directories(zpc_attribute_mapper PUBLIC include)

if(BUILD_TESTING)
  # mocks
  target_add_mock(zpc_attribute_mapper)

  # Test
  add_subdirectory(test)
endif()
