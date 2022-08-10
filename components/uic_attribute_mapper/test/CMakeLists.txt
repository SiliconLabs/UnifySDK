target_add_unittest(
  uic_attribute_mapper
  SOURCES attribute_mapper_test.cpp
  DEPENDS
  uic_config_mock
  uic_contiki_stub)

target_include_directories(uic_attribute_mapper_test PRIVATE ../src)

target_add_unittest(uic_attribute_mapper NAME uic_attribute_mapper_eval_test
                    SOURCES attribute_mapper_eval_test.cpp)
target_include_directories(uic_attribute_mapper_eval_test PRIVATE ../src)

target_add_unittest(uic_attribute_mapper NAME uic_attribute_mapper_uam_test
                    SOURCES attribute_mapper_uam_test.cpp)
target_include_directories(uic_attribute_mapper_uam_test PRIVATE ../src)

target_add_unittest(
  uic_attribute_mapper NAME uic_attribute_mapper_uam_test_example_1 SOURCES
  attribute_mapper_uam_test_example_1.cpp)
target_include_directories(uic_attribute_mapper_uam_test_example_1
                           PRIVATE ../src ../../uic_attribute_store/src)
target_link_libraries(uic_attribute_mapper_uam_test_example_1
                      PUBLIC uic_attribute_store_test_helpers)

# FIXME target_add_unittest(uic_attribute_mapper NAME
# uic_attribute_mapper_uam_test_example_2 SOURCES
# attribute_mapper_uam_test_example_2.cpp)
# target_include_directories(uic_attribute_mapper_uam_test_example_2 PRIVATE
# ../src ../../uic_attribute_store/src)
# target_link_libraries(uic_attribute_mapper_uam_test_example_2 PUBLIC
# uic_attribute_store_test_helpers)

## Attribute Mapper process test
target_add_unittest(
  uic_attribute_mapper
  NAME attribute_mapper_process_test
  SOURCES attribute_mapper_process_test.cpp
  DEPENDS
  uic_contiki_stub)

target_include_directories(attribute_mapper_process_test PRIVATE ../src)
