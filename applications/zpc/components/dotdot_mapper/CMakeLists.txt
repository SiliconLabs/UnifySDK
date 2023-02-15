# Binding cluster mapper helper
add_library(dotdot_mapper_binding_cluster_helper
            src/binding_cluster_mapper_helper.cpp)
target_include_directories(
  dotdot_mapper_binding_cluster_helper
  PUBLIC include
  PRIVATE src)
target_link_libraries(
  dotdot_mapper_binding_cluster_helper
  PUBLIC unify zwave_definitions
  PRIVATE zpc_attribute_store)

# DotDot Cluster Attribute Mappers
add_library(
  dotdot_mapper src/basic_cluster_mapper.cpp src/binding_cluster_mapper.c
                src/dotdot_mapper_fixt.c src/on_off_cluster_basic_mapper.cpp)

target_include_directories(
  dotdot_mapper
  PUBLIC include
  PRIVATE src test)

target_link_libraries(
  dotdot_mapper
  PUBLIC dotdot_mapper_binding_cluster_helper unify zpc_dotdot_mqtt
         zpc_attribute_store
  PRIVATE zwave_handlers ucl_definitions)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

# Copy rules to binary dir as they are runtime dependencies for the tests.
file(COPY rules DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
