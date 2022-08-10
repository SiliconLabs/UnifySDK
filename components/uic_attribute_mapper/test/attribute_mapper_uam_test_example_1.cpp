

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
#include "attribute_store_type_registration.h"
#include "datastore.h"
#include "attribute_mapper_engine.hpp"
#include "attribute_store_test_helpers.hpp"

extern "C" {

// Short alias for this namespace
using namespace std;
using namespace attribute_store;
using int_t = int32_t;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  attribute_store_register_type(0x3102,
                                "Multilevel Sensor Type",
                                0,
                                U8_STORAGE_TYPE);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  attribute::root().delete_node();
}

// Example with hex string as branch descriptor
void test_mapper_engine_example()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();

  // Create attribute tree
  create_branch("0x04020000").set_reported<int_t>(100);

  create_branch("0x3102[1].0x3103").set_reported<int_t>(0);
  create_branch("0x3102[1].0x3104").set_reported<int_t>(2);
  create_branch("0x3102[1].0x3105").set_reported<int_t>(100);

  create_branch("0x3102[0].0x3108[1].0x3104");
  create_branch("0x3102[0].0x3108[1].0x3105");

  create_branch("0x3102[2].0x3103").set_reported<int_t>(0);
  create_branch("0x3102[2].0x3104").set_reported<int_t>(2);
  create_branch("0x3102[2].0x3105").set_reported<int_t>(100);

  attribute_store_log();

  // Load UAM file
  // Assumes that file located in uic/components/uic_attribute_mapper/test/rules/
  TEST_ASSERT_TRUE(e.load_file("../../../../components/uic_attribute_mapper/"
                               "test/rules/complex_test_map_1.uam"));

  // Activate rules
  attribute_store_node_t updated_node
    = check_branch("0x3102[1].0x3105").set_reported<int_t>(10);
  e.on_attribute_updated(updated_node, REPORTED_ATTRIBUTE, ATTRIBUTE_UPDATED);

  check_branch("0x3102[2].0x3105").set_desired<int_t>(10);
  e.on_attribute_updated(updated_node, DESIRED_ATTRIBUTE, ATTRIBUTE_UPDATED);

  // Test
  TEST_ASSERT_EQUAL(1, check_branch("0x04020000").reported<int_t>());

  // This branch created as result of rules activation
  attribute node;
  node = check_branch("0x04020008");
  TEST_ASSERT_TRUE(node.is_valid());
  TEST_ASSERT_EQUAL(1, node.reported<uint32_t>());
}
}
