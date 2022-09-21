/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "workaround_for_test.hpp"
#include "attribute.hpp"
using namespace attribute_store;

extern "C" {
#include "unity.h"
#include "zpc_attribute_store.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "datastore.h"
#include "zwave_command_handler.h"
#include "zwave_controller_storage.h"
#include "zwave_command_class_battery.h"
#include "zwave_COMMAND_CLASS_BATTERY_attribute_id.h"
#include "zpc_attribute_store_test_helper.h"
#include "zpc_attribute_store_type_registration.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  // checking in the test that we store attributes consistently
  sl_status_t status = zpc_attribute_store_register_known_attribute_types();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
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
  zwave_unid_set_home_id(home_id);

  // adding required attributes for Battery Health Report parser test
  attribute attr_ep_node(endpoint_id_node);
  attr_ep_node.add_node(ATTRIBUTE_BATTERY_HEALTH_MAXIMUM_CAPACITY);
  attr_ep_node.add_node(ATTRIBUTE_BATTERY_HEALTH_BATTERY_TEMPERATURE);
  attr_ep_node.add_node(ATTRIBUTE_BATTERY_HEALTH_SCALE);
  attr_ep_node.add_node(ATTRIBUTE_BATTERY_HEALTH_PRECISION);

  zwave_command_class_battery_control_init();
}

void test_zwave_command_class_battery_test()
{
  attribute attr_ep_node(endpoint_id_node);
  zwave_controller_connection_info_t connection_info
    = {.remote = {.node_id = 4, .endpoint_id = 3}};

  // Testing if the battery health report is parsed correctly
  const uint8_t battery_health_report[] = {0x80, 0x05, 0xFF, 0x01, 0x00};
  zwave_command_handler_dispatch(&connection_info,
                                 battery_health_report,
                                 sizeof(battery_health_report));

  attribute max_capacity
    = attr_ep_node.child_by_type(ATTRIBUTE_BATTERY_HEALTH_MAXIMUM_CAPACITY);
  TEST_ASSERT_TRUE(max_capacity.is_valid());
  uint8_t max_capacity_value = max_capacity.reported<uint8_t>();
  TEST_ASSERT_EQUAL(0xFF, max_capacity_value);

  attribute temp
    = attr_ep_node.child_by_type(ATTRIBUTE_BATTERY_HEALTH_BATTERY_TEMPERATURE);
  int32_t temp_value = temp.reported<int32_t>();
  TEST_ASSERT_EQUAL(0x00, temp_value);

  attribute scale = attr_ep_node.child_by_type(ATTRIBUTE_BATTERY_HEALTH_SCALE);
  uint8_t scale_value = scale.reported<uint8_t>();
  TEST_ASSERT_EQUAL(0x00, scale_value);

  attribute precision
    = attr_ep_node.child_by_type(ATTRIBUTE_BATTERY_HEALTH_PRECISION);
  uint8_t precision_value = precision.reported<uint8_t>();
  TEST_ASSERT_EQUAL(0x00, precision_value);

  const uint8_t battery_health_report_2[]
    = {0x80, 0x05, 0xFF, 0x02, 0x01, 0x2C};
  zwave_command_handler_dispatch(&connection_info,
                                 battery_health_report_2,
                                 sizeof(battery_health_report_2));
  int32_t temperature_value = temp.reported<int32_t>();
  TEST_ASSERT_EQUAL_INT32(300, temperature_value);

  temp.delete_node();
  zwave_command_handler_dispatch(&connection_info,
                                 battery_health_report_2,
                                 sizeof(battery_health_report_2));
}
}