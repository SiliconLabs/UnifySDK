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
#include "dotdot_attributes.h"

// Interfaces
#include "zwave_generic_types.h"
#include "zwave_command_class_version_types.h"

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

void test_power_source_mapper()
{
  attribute listening_node
    = attribute(node_id_node).add_node(ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING);
  TEST_ASSERT_TRUE(listening_node.is_valid());
  attribute optional_node
    = attribute(node_id_node).add_node(ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL);
  TEST_ASSERT_TRUE(optional_node.is_valid());

  // Set reported values
  listening_node.set_reported<uint8_t>(0x80);
  optional_node.set_reported<uint8_t>(0x00);

  // AL node mapped to 4
  attribute power_source_node
    = attribute(endpoint_id_node)
        .child_by_type(DOTDOT_ATTRIBUTE_ID_BASIC_POWER_SOURCE);
  TEST_ASSERT_TRUE(power_source_node.is_valid());

  TEST_ASSERT_EQUAL(4, power_source_node.reported<int32_t>());

  // Set it to FL node:
  listening_node.set_reported<uint8_t>(0x00);
  optional_node.set_reported<uint8_t>(0x40);
  TEST_ASSERT_EQUAL(3, power_source_node.reported<int32_t>());

  // Set it to NL node:
  listening_node.set_reported<uint8_t>(0x00);
  optional_node.set_reported<uint8_t>(0x40);
  TEST_ASSERT_EQUAL(3, power_source_node.reported<int32_t>());

  // Set it to Unknown:
  attribute_store_delete_node(optional_node);
  listening_node.set_reported<uint8_t>(0x01);
  TEST_ASSERT_EQUAL(0, power_source_node.reported<int32_t>());
}

void test_hardware_version_mapper()
{
  attribute hardware_node = attribute(endpoint_id_node)
                              .add_node(ATTRIBUTE_CC_VERSION_HARDWARE_VERSION);
  TEST_ASSERT_TRUE(hardware_node.is_valid());

  // Set reported values
  hardware_node.set_reported<zwave_version_hardware_version_t>(
    (zwave_version_hardware_version_t)235);

  // See that it mapped to the same value.
  attribute basic_hardware_version_node
    = attribute(endpoint_id_node)
        .child_by_type(DOTDOT_ATTRIBUTE_ID_BASIC_HW_VERSION);
  TEST_ASSERT_TRUE(basic_hardware_version_node.is_valid());

  TEST_ASSERT_EQUAL(235, basic_hardware_version_node.reported<int32_t>());
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

  attribute dotdot_serial_node
    = attribute(endpoint_id_node)
        .child_by_type(DOTDOT_ATTRIBUTE_ID_BASIC_SERIAL_NUMBER);

  TEST_ASSERT_TRUE(dotdot_serial_node.reported<std::string>()
                   == "h'112233445566");

  //Now check is format is UTF 8
  const char hello_world[] = "HelloWorld";
  device_id_format_node.set_reported<uint32_t>(0x0);
  //Store the string without the 0 termination character
  device_id_data_node.set(REPORTED_ATTRIBUTE,
                          (const uint8_t *)hello_world,
                          sizeof(hello_world) - 1);
  TEST_ASSERT_TRUE(dotdot_serial_node.reported<std::string>() == "HelloWorld");
}
}
