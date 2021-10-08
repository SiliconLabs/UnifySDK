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
#include "zwave_command_class_indicator_control.h"
#include "zpc_attribute_store.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "datastore.h"
#include "zwave_command_handler.h"
#include "attribute_resolver.h"

#include "attribute.hpp"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_COMMAND_CLASS_INDICATOR_attribute_id.h"

using namespace attribute_store;

#include "workaround_for_test.hpp"
static std::map<attribute_store_type_t, attribute_resolver_function_t> get_rule;
static std::map<attribute_store_type_t, attribute_resolver_function_t> set_rule;

extern "C" {
#include "zwave_network_management_mock.h"
#include "attribute_resolver_mock.h"

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
void setUp() {}

static sl_status_t
  attribute_resolver_register_rule_mock(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  set_rule.insert(std::make_pair(node_type, set_func));
  get_rule.insert(std::make_pair(node_type, get_func));
  return SL_STATUS_OK;
}

void test_zwave_command_class_indicator_control_test()
{
  zwave_network_management_get_home_id_IgnoreAndReturn(0x11111111);
  zwave_network_management_get_node_id_IgnoreAndReturn(1);

  zwave_command_handler_init();

  zpc_attribute_store_init();

  zwave_controller_connection_info_t connection_info
    = {.remote = {.node_id = 5, .endpoint_id = 0}};

  attribute n5ep0
    = attribute::root()
        .emplace_node<zwave_home_id_t>(ATTRIBUTE_HOME_ID, 0x11111111)
        .emplace_node<zwave_node_id_t>(ATTRIBUTE_NODE_ID, 5)
        .emplace_node<zwave_endpoint_id_t>(ATTRIBUTE_ENDPOINT_ID, 0);
  zwave_unid_set_home_id(0x11111111);

  //Indicator stuff
  attribute_resolver_set_attribute_depth_ExpectAndReturn(
    ATTRIBUTE_INDICATOR_INDICATOR_VALUE,
    2,
    SL_STATUS_OK);
  attribute_resolver_register_rule_Stub(attribute_resolver_register_rule_mock);
  //attribute_resolver_register_rule_IgnoreAndReturn(SL_STATUS_OK);

  zwave_command_class_indicator_control_init();

  // Now add the indicator version
  n5ep0.emplace_node<uint8_t>(ATTRIBUTE_COMMAND_CLASS_INDICATOR_VERSION, 2);

  // Test that we create indicator 0 initially
  attribute indicator
    = n5ep0.child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_INDICATOR_ID,
                                             0);
  TEST_ASSERT_TRUE(indicator.is_valid());
  TEST_ASSERT_TRUE(
    indicator
      .child_by_type(ATTRIBUTE_INDICATOR_SUPPORTED_PROPERTY_SUPPORTED_BIT_MASK)
      .is_valid());

  uint8_t buffer[255];
  uint16_t length;
  sl_status_t rc;

  //Check that we will not make a indicator get frame with indicator 0
  rc
    = get_rule[ATTRIBUTE_INDICATOR_INDICATOR_VALUE](indicator, buffer, &length);
  TEST_ASSERT_EQUAL(SL_STATUS_IS_WAITING, rc);

  //Check that we can make a proper get capabilities get frame,
  const uint8_t indicator0_supported_get[] = {0x87, 0x04, 0x00};
  rc = get_rule[ATTRIBUTE_INDICATOR_SUPPORTED_PROPERTY_SUPPORTED_BIT_MASK](
    indicator.child_by_type(
      ATTRIBUTE_INDICATOR_SUPPORTED_PROPERTY_SUPPORTED_BIT_MASK),
    buffer,
    &length);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, rc);
  TEST_ASSERT_EQUAL(3, length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(indicator0_supported_get, buffer, 3);

  //Lets inject supported report with indicator 5, next_indicator = 0x50
  // and support for property 0 and 4
  const uint8_t indicator0_supported_report[] = {0x87, 0x05, 0x05, 0x50, 1, 6};

  zwave_command_handler_dispatch(&connection_info,
                                 indicator0_supported_report,
                                 sizeof(indicator0_supported_report));

  TEST_ASSERT_EQUAL(0x5, indicator.reported<uint8_t>());
  TEST_ASSERT_TRUE(
    indicator
      .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 1)
      .is_valid());
  TEST_ASSERT_TRUE(
    indicator
      .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 2)
      .is_valid());

  //Did we get indicator 50 added ?
  attribute indicator50
    = n5ep0.child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_INDICATOR_ID,
                                             0x50);
  TEST_ASSERT_TRUE(indicator50.is_valid());
  TEST_ASSERT_TRUE(
    indicator50
      .child_by_type(ATTRIBUTE_INDICATOR_SUPPORTED_PROPERTY_SUPPORTED_BIT_MASK)
      .is_valid());

  //Can we do a supported get of indicator 50
  const uint8_t indicator50_supported_get[] = {0x87, 0x04, 0x50};
  rc = get_rule[ATTRIBUTE_INDICATOR_SUPPORTED_PROPERTY_SUPPORTED_BIT_MASK](
    indicator50.child_by_type(
      ATTRIBUTE_INDICATOR_SUPPORTED_PROPERTY_SUPPORTED_BIT_MASK),
    buffer,
    &length);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, rc);
  TEST_ASSERT_EQUAL(3, length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(indicator50_supported_get, buffer, 3);

  //Lets fire a supported report on indicator 50, with no more indicators, property 1 and 3
  const uint8_t indicator50_supported_report[]
    = {0x87, 0x05, 0x050, 0x00, 1, 10};
  zwave_command_handler_dispatch(&connection_info,
                                 indicator50_supported_report,
                                 sizeof(indicator0_supported_report));
  TEST_ASSERT_TRUE(
    indicator50
      .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 1)
      .is_valid());
  TEST_ASSERT_TRUE(
    indicator50
      .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 3)
      .is_valid());

  //Can we do an indicator get of indicator5?
  auto val5_1
    = indicator
        .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 1)
        .child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE);
  const uint8_t indicator5_get[] = {0x87, 0x02, 0x05};
  rc = get_rule[ATTRIBUTE_INDICATOR_INDICATOR_VALUE](val5_1, buffer, &length);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, rc);
  TEST_ASSERT_EQUAL(3, length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(indicator5_get, buffer, 3);

  //Can we do an indicator get of indicator50?
  auto val50_1
    = indicator50
        .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 1)
        .child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE);
  const uint8_t indicator50_get[] = {0x87, 0x02, 0x50};
  rc = get_rule[ATTRIBUTE_INDICATOR_INDICATOR_VALUE](val50_1, buffer, &length);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, rc);
  TEST_ASSERT_EQUAL(3, length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(indicator50_get, buffer, 3);

  //Lets report for indicator 5
  //attribute_store_log_node(indicator,true);
  const uint8_t indicator5_report[]
    = {0x87, 0x03, 0x32, 0x2, 0x5, 0x1, 0xaa, 0x5, 0x2, 0xbb};
  zwave_command_handler_dispatch(&connection_info,
                                 indicator5_report,
                                 sizeof(indicator5_report));
  TEST_ASSERT_EQUAL(
    0xaa,
    indicator
      .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 1)
      .child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE)
      .reported<int32_t>());
  TEST_ASSERT_EQUAL(
    0xbb,
    indicator
      .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 2)
      .child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE)
      .reported<int32_t>());

  //Lets report for indicator 5 but report a false parameter
  //attribute_store_log_node(indicator,true);
  const uint8_t indicator5_false_report[]
    = {0x87, 0x03, 0x32, 0x2, 0x5, 0x11, 0xcc, 0x5, 0x2, 0xdd};
  zwave_command_handler_dispatch(&connection_info,
                                 indicator5_false_report,
                                 sizeof(indicator5_false_report));
  TEST_ASSERT_EQUAL(
    0x00,
    indicator
      .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 1)
      .child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE)
      .reported<int32_t>());
  TEST_ASSERT_EQUAL(
    0xdd,
    indicator
      .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 2)
      .child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE)
      .reported<int32_t>());

  //Lets report for indicator 5 but cut the frame short
  const uint8_t indicator5_false_report2[]
    = {0x87, 0x03, 0x32, 0x2, 0x5, 0x1, 0xee, 0x5, 0x2};
  zwave_command_handler_dispatch(&connection_info,
                                 indicator5_false_report2,
                                 sizeof(indicator5_false_report2));
  TEST_ASSERT_EQUAL(
    0x00,
    indicator
      .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 1)
      .child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE)
      .reported<int32_t>());
  TEST_ASSERT_EQUAL(
    0xdd,
    indicator
      .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 2)
      .child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE)
      .reported<int32_t>());

  //Can we do a set of indicator 50 ?
  const uint8_t indicator50_set[]
    = {0x87, 0x01, 0xff, 0x2, 0x50, 0x1, 0xaa, 0x50, 0x3, 0xbb};

  auto indicator50_1_value
    = indicator50
        .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 1)
        .child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE);
  indicator50_1_value.set_desired<int32_t>(0xaa);
  indicator50
    .child_by_type_and_value<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, 3)
    .child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE)
    .set_desired<int32_t>(0xbb);

  //attribute_store_log_node(indicator50, true);

  rc = set_rule[ATTRIBUTE_INDICATOR_INDICATOR_VALUE](indicator50_1_value,
                                                     buffer,
                                                     &length);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, rc);
  TEST_ASSERT_EQUAL_INT8_ARRAY(indicator50_set,
                               buffer,
                               sizeof(indicator50_set));
  TEST_ASSERT_EQUAL(sizeof(indicator50_set), length);
}
}
