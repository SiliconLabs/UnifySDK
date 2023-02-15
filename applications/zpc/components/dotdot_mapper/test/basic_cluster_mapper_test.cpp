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
#include "basic_cluster_mapper.h"

#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_COMMAND_CLASS_MANUFACTURER_SPECIFIC_attribute_id.h"
#include "attribute.hpp"
#include "zpc_attribute_store_test_helper.h"
#include "unify_dotdot_defined_attribute_types.h"

// Interfaces
#include "zwave_generic_types.h"
#include "zwave_command_class_version_types.h"
#include "ucl_definitions.h"

#include "workaround_basic_cluster_mapper_test.hpp"
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
  basic_cluster_mapper_init();
}

void test_manufacturer_id_to_name_mapper()
{
  // Create ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID
  // and DOTDOT_ATTRIBUTE_ID_BASIC_MANUFACTURER_NAME attributes
  attribute attr_ep_node(endpoint_id_node);
  attr_ep_node.add_node(ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID);
  attr_ep_node.add_node(DOTDOT_ATTRIBUTE_ID_BASIC_MANUFACTURER_NAME);

  attribute manufacturer_id_node = attr_ep_node.child_by_type(
    ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID);
  TEST_ASSERT_TRUE(manufacturer_id_node.is_valid());
  // Set reported value
  //{0x0000, "Z-Wave Protocol Owner"},
  manufacturer_id_node.set_reported<uint32_t>(0x0000);

  attribute attr_node_id_node(node_id_node);
  attr_node_id_node.add_node(ATTRIBUTE_NETWORK_STATUS)
    .set_reported<node_state_topic_state_t>(NODE_STATE_TOPIC_STATE_INCLUDED);

  attribute basic_cluster_man_name_node
    = attr_ep_node.child_by_type(DOTDOT_ATTRIBUTE_ID_BASIC_MANUFACTURER_NAME);
  TEST_ASSERT_TRUE(basic_cluster_man_name_node.is_valid());

  std::vector<uint8_t> v
    = basic_cluster_man_name_node.reported<std::vector<uint8_t>>();
  v.pop_back();
  std::string manufacturer_name(v.begin(), v.end());
  TEST_ASSERT_TRUE(manufacturer_name == "Z-Wave Protocol Owner");
}

void test_serial_number_mapper()
{
  attribute device_id_type_node
    = attribute(endpoint_id_node)
        .emplace_node<uint8_t>(ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_TYPE, 0x1);

  attribute device_id_format_node = device_id_type_node.add_node(
    ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_DATA_FORMAT);
  attribute device_id_data_node
    = device_id_type_node.add_node(ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_DATA);

  //First check if format is binary ie equal to 1
  device_id_format_node.set_reported<uint32_t>(0x1);
  device_id_data_node.set_reported(
    std::vector<uint8_t>({0x11, 0x22, 0x33, 0x44, 0x55, 0x66}));

  attribute attr_node_id_node(node_id_node);
  attr_node_id_node.add_node(ATTRIBUTE_NETWORK_STATUS)
    .set_reported<node_state_topic_state_t>(NODE_STATE_TOPIC_STATE_INCLUDED);

  attribute dotdot_serial_node
    = attribute(endpoint_id_node)
        .child_by_type(DOTDOT_ATTRIBUTE_ID_BASIC_SERIAL_NUMBER);

  char stored_string[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  attribute_store_get_reported_string(dotdot_serial_node,
                                      stored_string,
                                      sizeof(stored_string));
  TEST_ASSERT_EQUAL_STRING("h'112233445566", stored_string);
}
}
