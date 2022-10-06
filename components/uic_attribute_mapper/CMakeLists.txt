find_package(Boost COMPONENTS filesystem)

add_library(
  uic_attribute_mapper
  src/attribute_mapper.cpp
  src/attribute_mapper_ast.cpp
  src/attribute_mapper_ast_path_eval.cpp
  src/attribute_mapper_ast_dep_eval.cpp
  src/attribute_mapper_cyclic_check.cpp
  src/attribute_mapper_engine.cpp
  src/attribute_mapper_process.cpp
  src/attribute_mapper_parser.cpp)

target_link_libraries(
  uic_attribute_mapper
  #this_fixes_a_bug # cmake omits the first library in this list if you remove this library
  uic_log 
  uic_config
  uic_attribute_store
  uic_contiki
  uic_contiki_platform
  uic_datastore
  Boost::filesystem
  unify_version)
target_include_directories(uic_attribute_mapper PUBLIC include)

install_headers(uic_attribute_mapper "uic" "uic-dev")

add_executable(uic_mapper_test src/attribute_mapper_test_framework.cpp)
target_link_libraries(
  uic_mapper_test
  PUBLIC uic_log uic_config uic_attribute_store uic_attribute_mapper
         uic_attribute_store_test_helpers Boost::filesystem)
target_include_directories(
  uic_mapper_test PUBLIC include ${CMAKE_BINARY_DIR}/include
                         ../uic_attribute_store/src)

add_subdirectory(test)
