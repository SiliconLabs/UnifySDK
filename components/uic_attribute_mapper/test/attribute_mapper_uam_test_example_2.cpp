

/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "unity.h"
#include "attribute_store_fixt.h"
#include "datastore.h"
#include "workaround.hpp"
#include "attribute_store_type_registration.h"
#include "uic_version.h"

extern "C" {

// Short alias for this namespace
using namespace std;
using namespace attribute_store;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  attribute_store_register_type(0x04020000,
                                "ZCLMeasuredValue",
                                ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                U32_STORAGE_TYPE);
  attribute_store_register_type(0x3102,
                                "SensorType",
                                ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                U8_STORAGE_TYPE);
  attribute_store_register_type(0x3103,
                                "Scale",
                                ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                U32_STORAGE_TYPE);
  attribute_store_register_type(0x3104,
                                "Precision",
                                ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                U32_STORAGE_TYPE);
  attribute_store_register_type(0x3105,
                                "Value",
                                ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                U32_STORAGE_TYPE);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  attribute::root().delete_node();
}

// Example with string as branch descriptor
void test_mapper_engine_example()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  // Create attribute tree
  create_branch("ZCLMeasuredValue").set_reported<uint32_t>(100);

  create_branch("SensorType[1].Scale").set_reported<uint32_t>(0);
  create_branch("SensorType[1].Precision").set_reported<uint32_t>(2);
  create_branch("SensorType[1].Value").set_reported<uint32_t>(10);

  create_branch("SensorType[2].Scale").set_reported<uint32_t>(1);
  create_branch("SensorType[2].Precision").set_reported<uint32_t>(2);
  create_branch("SensorType[2].Value").set_reported<uint32_t>(10);

  // Load UAM file
  TEST_ASSERT_TRUE(e.load_file(SOURCE_DIR "/components/uic_attribute_mapper/"
                                          "test/rules/complex_test_map_2.uam"));

  // Activate rules
  attribute_store_node_t updated_node
    = check_branch("SensorType[1].Value").set_reported<uint32_t>(100);
  TEST_ASSERT_EQUAL(100, check_branch("ZCLMeasuredValue").reported<uint32_t>());

  // Test
  e.on_attribute_updated(updated_node, REPORTED_ATTRIBUTE, ATTRIBUTE_UPDATED);
  TEST_ASSERT_EQUAL(10, check_branch("ZCLMeasuredValue").reported<uint32_t>());
}

// Example with string as branch descriptor
void test_mapper_engine_example_()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  // Create attribute tree
  create_branch("ZCLMeasuredValue").set_reported<uint32_t>(100);

  create_branch("SensorType[1].Scale").set_reported<uint32_t>(1);
  create_branch("SensorType[1].Precision").set_reported<uint32_t>(2);
  create_branch("SensorType[1].Value").set_reported<uint32_t>(100);

  create_branch("SensorType[2].Scale").set_reported<uint32_t>(1);
  create_branch("SensorType[2].Precision").set_reported<uint32_t>(2);
  create_branch("SensorType[2].Value").set_reported<uint32_t>(100);

  // Load UAM file
  TEST_ASSERT_TRUE(e.load_file(SOURCE_DIR "/components/uic_attribute_mapper/"
                                          "test/rules/complex_test_map_2.uam"));

  // Activate rules
  attribute_store_node_t updated_node
    = check_branch("SensorType[1].Value").set_reported<uint32_t>(167000);
  e.on_attribute_updated(updated_node, REPORTED_ATTRIBUTE, ATTRIBUTE_UPDATED);

  TEST_ASSERT_EQUAL(750, check_branch("ZCLMeasuredValue").reported<uint32_t>());
}
}
