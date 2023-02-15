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
// Test includes
#include "unity.h"
#include "ucl_mqtt_node_interview.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "attribute_resolver.h"
#include "attribute_store_defined_attribute_types.h"

const static unid_t not_valid_unid = "Not a UNID";

// Test function, stub get resolution function
sl_status_t get_resolution_function(attribute_store_node_t node,
                                    uint8_t *frame,
                                    uint16_t *frame_length)
{
  *frame_length = 0;
  return SL_STATUS_OK;
}
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);
}

/// Called after each and every test
void tearDown()
{
  attribute_store_teardown();
  datastore_teardown();
}

void test_ucl_mqtt_initiate_node_interview_happy_case()
{
  // Create a few attributes under the endpoint
  // clang-format off
  attribute_store_node_t attribute_1 = attribute_store_add_node(0x1001,endpoint_id_node);
  attribute_store_node_t attribute_2 = attribute_store_add_node(0x1002,attribute_1);
  attribute_store_node_t attribute_3 = attribute_store_add_node(0x1003,attribute_1);
  attribute_store_node_t attribute_4 = attribute_store_add_node(0x1004,attribute_3);
  attribute_store_node_t attribute_5 = attribute_store_add_node(0x1005,attribute_2);
  // clang-format on

  attribute_resolver_register_rule(0x1001, NULL, NULL);
  attribute_resolver_register_rule(0x1002, NULL, &get_resolution_function);
  attribute_resolver_register_rule(0x1003, NULL, &get_resolution_function);
  attribute_resolver_register_rule(0x1004, NULL, &get_resolution_function);
  attribute_resolver_register_rule(0x1005, NULL, &get_resolution_function);

  uint8_t value = 1;
  attribute_store_set_reported(attribute_1, &value, sizeof(value));
  attribute_store_set_reported(attribute_2, &value, sizeof(value));
  attribute_store_set_reported(attribute_3, &value, sizeof(value));
  attribute_store_set_reported(attribute_4, &value, sizeof(value));
  attribute_store_set_reported(attribute_5, &value, sizeof(value));
  attribute_store_log();

  // clang-format off
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_1, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_3, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_4, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_5, REPORTED_ATTRIBUTE));
  // clang-format on

  TEST_ASSERT_EQUAL(SL_STATUS_OK, ucl_mqtt_initiate_node_interview(zpc_unid));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    ucl_mqtt_initiate_node_interview(supporting_node_unid));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    ucl_mqtt_initiate_node_interview(not_valid_unid));

  // clang-format off
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_1, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(attribute_2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(attribute_3, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(attribute_4, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(attribute_5, REPORTED_ATTRIBUTE));
  // clang-format on
}

void test_ucl_mqtt_initiate_endpoint_interview()
{
  // Create a few attributes under the endpoint
  // clang-format off
  attribute_store_node_t attribute_1 = attribute_store_add_node(0x1001,endpoint_id_node);
  attribute_store_node_t attribute_2 = attribute_store_add_node(0x1002,attribute_1);
  attribute_store_node_t attribute_3 = attribute_store_add_node(0x1003,attribute_1);
  attribute_store_node_t attribute_4 = attribute_store_add_node(0x1004,attribute_3);
  attribute_store_node_t attribute_5 = attribute_store_add_node(0x1005,attribute_2);
  // clang-format on

  attribute_resolver_register_rule(0x1001, NULL, NULL);
  attribute_resolver_register_rule(0x1002, NULL, &get_resolution_function);
  attribute_resolver_register_rule(0x1003, NULL, &get_resolution_function);
  attribute_resolver_register_rule(0x1004, NULL, &get_resolution_function);
  attribute_resolver_register_rule(0x1005, NULL, &get_resolution_function);

  uint8_t value = 1;
  attribute_store_set_reported(attribute_1, &value, sizeof(value));
  attribute_store_set_reported(attribute_2, &value, sizeof(value));
  attribute_store_set_reported(attribute_3, &value, sizeof(value));
  attribute_store_set_reported(attribute_4, &value, sizeof(value));
  attribute_store_set_reported(attribute_5, &value, sizeof(value));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    ucl_mqtt_initiate_endpoint_interview(zpc_unid, 0));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    ucl_mqtt_initiate_endpoint_interview(supporting_node_unid, 2));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    ucl_mqtt_initiate_endpoint_interview(not_valid_unid, 3));

  // clang-format off
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_1, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_3, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_4, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_5, REPORTED_ATTRIBUTE));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    ucl_mqtt_initiate_endpoint_interview(supporting_node_unid, endpoint_id));

  // clang-format off
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_1, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(attribute_2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(attribute_3, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(attribute_4, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(attribute_5, REPORTED_ATTRIBUTE));
  // clang-format on
}
