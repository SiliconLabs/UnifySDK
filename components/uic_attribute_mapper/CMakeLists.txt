find_package(Boost COMPONENTS filesystem)

add_library(
  uic_attribute_mapper OBJECT
  src/attribute_mapper.cpp
  src/attribute_mapper_ast.cpp
  src/attribute_mapper_ast_path_eval.cpp
  src/attribute_mapper_ast_dep_eval.cpp
  src/attribute_mapper_cyclic_check.cpp
  src/attribute_mapper_engine.cpp
  src/attribute_mapper_process.cpp
  src/attribute_mapper_parser.cpp
  src/attribute_mapper_built_in_functions.cpp
  src/attribute_mapper_scope_settings.cpp)

target_link_libraries(
  uic_attribute_mapper
  PRIVATE uic_log
          uic_config
          uic_attribute_store
          uic_contiki
          uic_contiki_platform
          uic_datastore
          Boost::filesystem
          unify_version)

target_include_directories(uic_attribute_mapper
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>)

install_headers(uic_attribute_mapper uic libuic-dev)

if(BUILD_TESTING)
  add_executable(uic_mapper_test src/attribute_mapper_test_framework.cpp)
  target_link_libraries(
    uic_mapper_test PRIVATE unify Boost::filesystem
                            uic_attribute_store_test_helpers)

  target_include_directories(
    uic_mapper_test PUBLIC include ${CMAKE_BINARY_DIR}/include
                           ../uic_attribute_store/src ../src)

  # Mocks
  target_add_mock(uic_attribute_mapper)

  add_subdirectory(test)
endif()
