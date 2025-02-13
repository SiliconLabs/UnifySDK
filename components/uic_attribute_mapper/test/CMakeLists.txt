add_library(attribute_mapper_shared SHARED
            $<TARGET_OBJECTS:uic_attribute_mapper>)
target_link_libraries(attribute_mapper_shared unify Boost::filesystem)

target_include_directories(
  attribute_mapper_shared PRIVATE ../../uic_attribute_store/src ../src
                                  ../include)
target_add_unittest(
  attribute_mapper_shared
  NAME
  uic_attribute_mapper_test
  SOURCES
  attribute_mapper_test.cpp
  DEPENDS
  uic_config_mock
  uic_contiki_stub)

target_add_unittest(attribute_mapper_shared NAME uic_attribute_mapper_eval_test
                    SOURCES attribute_mapper_eval_test.cpp)
target_include_directories(uic_attribute_mapper_eval_test PRIVATE ../src)

#target_add_unittest(attribute_mapper_shared NAME uic_attribute_mapper_uam_test
#                    SOURCES attribute_mapper_uam_test.cpp)
#target_include_directories(uic_attribute_mapper_uam_test PRIVATE ../src)

target_add_unittest(
  attribute_mapper_shared NAME uic_attribute_mapper_uam_test_example_1 SOURCES
  attribute_mapper_uam_test_example_1.cpp)
target_include_directories(uic_attribute_mapper_uam_test_example_1
                           PRIVATE ../src ../../uic_attribute_store/src)
target_link_libraries(uic_attribute_mapper_uam_test_example_1
                      PUBLIC uic_attribute_store_test_helpers)

target_add_unittest(
  attribute_mapper_shared NAME uic_attribute_mapper_uam_test_example_2 SOURCES
  attribute_mapper_uam_test_example_2.cpp)
target_include_directories(uic_attribute_mapper_uam_test_example_2
                           PRIVATE ../src ../../uic_attribute_store/src)
target_link_libraries(uic_attribute_mapper_uam_test_example_2
                      PUBLIC uic_attribute_store_test_helpers)

# Attribute Mapper process test
target_add_unittest(
  attribute_mapper_shared
  NAME
  attribute_mapper_process_test
  SOURCES
  attribute_mapper_process_test.cpp
  DEPENDS
  uic_contiki_stub)

target_include_directories(attribute_mapper_process_test PRIVATE ../src)

# Copy rules to binary dir as they are runtime dependencies for the tests.
file(COPY rules DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
