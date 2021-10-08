# Mappers are tested in the mapping engine

# Basic Cluster Mapper test
target_add_unittest(dotdot_mapper
  NAME
    manufacturer_id_to_name_mapper_test
  SOURCES
    manufacturer_id_to_name_mapper_test.cpp
  DEPENDS
    zpc_attribute_store_test_helper
    dotdot_attributes
    uic_attribute_store
    zpc_attribute_store
)