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
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute.hpp"
#include "zpc_attribute_store_test_helper.h"
#include "dotdot_attributes.h"

#include "manufacturer_id_to_name_mapper.h"
#include "workaround_manufacturer_mapper_test.hpp"
using namespace attribute_store;

extern "C" {

#include "unity.h"
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
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
  zpc_attribute_store_test_helper_create_network();
  manufacturer_id_to_name_mapper_init();

  // Create ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID
  // and DOTDOT_ATTRIBUTE_ID_BASIC_MANUFACTURER_NAME attributes
  attribute attr_ep_node(endpoint_id_node);
  attr_ep_node.add_node(ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID);
  attr_ep_node.add_node(DOTDOT_ATTRIBUTE_ID_BASIC_MANUFACTURER_NAME);
}

void test_manufacturer_id_to_name_mapper()
{
  attribute manfacturer_id_node
    = attribute(endpoint_id_node)
        .child_by_type(ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID);
  TEST_ASSERT_TRUE(manfacturer_id_node.is_valid());

  // Set reported value
  //{0x0000, "Z-Wave Protocol Owner"},
  manfacturer_id_node.set_reported<uint32_t>(0x0000);

  attribute basic_cluster_manf_name_node
    = attribute(endpoint_id_node)
        .child_by_type(DOTDOT_ATTRIBUTE_ID_BASIC_MANUFACTURER_NAME);
  TEST_ASSERT_TRUE(basic_cluster_manf_name_node.is_valid());

  std::vector<uint8_t> v
    = basic_cluster_manf_name_node.reported<std::vector<uint8_t>>();
  v.pop_back();
  std::string manufacturer_name(v.begin(), v.end());

  TEST_ASSERT_TRUE(manufacturer_name == "Z-Wave Protocol Owner");

  // Test with Unknown manufacturer id
  manfacturer_id_node.set_reported<uint32_t>(0x89457);
  std::vector<uint8_t> v_test
    = basic_cluster_manf_name_node.reported<std::vector<uint8_t>>();
  v_test.pop_back();
  std::string test_unknown_manufacturer_name(v_test.begin(), v_test.end());

  TEST_ASSERT_TRUE(test_unknown_manufacturer_name == "Unknown Manufacturer");
}
}
