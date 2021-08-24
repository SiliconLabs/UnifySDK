find_package(Boost COMPONENTS filesystem)

add_library(uic_attribute_mapper SHARED src/attribute_mapper.cpp
src/attribute_mapper_engine.cpp 
src/attribute_mapper_ast.cpp
src/attribute_mapper_parser.cpp
src/attribute_mapper_ast_eval.cpp
src/attribute_mapper_ast_dep_eval.cpp)

target_link_libraries(uic_attribute_mapper PUBLIC uic_log uic_config uic_datastore uic_attribute_store Boost::filesystem)
target_include_directories(uic_attribute_mapper PUBLIC include ${CMAKE_BINARY_DIR}/include)

add_executable(uic_mapper_test src/attribute_mapper_test_framework.cpp)
target_link_libraries(uic_mapper_test PUBLIC uic_log uic_config uic_datastore uic_attribute_store uic_attribute_mapper uic_attribute_store_test_helpers Boost::filesystem)
target_include_directories(uic_mapper_test PUBLIC include ${CMAKE_BINARY_DIR}/include ../uic_attribute_store/src)

add_subdirectory(test)
