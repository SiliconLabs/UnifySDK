/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_attribute_store.h"

#include <string.h>

static int callback_counter = 0;

void test_callback()
{
  callback_counter++;
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
  TEST_ASSERT_EQUAL(SL_STATUS_OK, unify_dotdot_attribute_store_init());
  callback_counter = 0;
}

void test_unify_dotdot_attribute_store_configuration_test_null_functions()
{
  // Try to configure functions with NULL, see what happens

  unify_dotdot_attribute_store_configuration_t test_configuration = {
    .get_endpoint_node_function                  = NULL,
    .get_unid_endpoint_function                  = NULL,
    .get_unid_function                           = NULL,
    .update_attribute_desired_values_on_commands = false,
    .clear_reported_on_desired_updates           = false,
    .automatic_deduction_of_supported_commands   = false,
    .force_read_attributes_enabled               = false,
    .write_attributes_enabled                    = false,
    .publish_desired_attribute_values_to_mqtt    = false,
    .publish_reported_attribute_values_to_mqtt   = false,
    .node_type     = ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
    .endpoint_type = ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
  };

  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Now try to invoke these functions:
  dotdot_unid_t unid               = "test";
  dotdot_endpoint_id_t endpoint_id = 4;
  TEST_ASSERT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    unify_dotdot_attributes_get_endpoint_node()(unid, endpoint_id));

  char test_unid[MAXIMUM_UNID_SIZE] = "test";
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    unify_dotdot_attributes_get_unid_endpoint()(23, test_unid, &endpoint_id));
  TEST_ASSERT_EQUAL_STRING("", test_unid);
  strncpy(test_unid, "new unid value", MAXIMUM_UNID_SIZE);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    unify_dotdot_attributes_get_unid()(23, test_unid));
  TEST_ASSERT_EQUAL_STRING("", test_unid);
}

void test_unify_dotdot_attribute_store_configuration_callback()
{
  // Attach a callback for configuration changes:
  unify_dotdot_attribute_store_set_configuration_update_callback(
    &test_callback);

  TEST_ASSERT_EQUAL(0, callback_counter);

  // Try to configure functions with NULL, see what happens
  unify_dotdot_attribute_store_configuration_t test_configuration = {
    .get_endpoint_node_function                  = NULL,
    .get_unid_endpoint_function                  = NULL,
    .get_unid_function                           = NULL,
    .update_attribute_desired_values_on_commands = false,
    .clear_reported_on_desired_updates           = false,
    .automatic_deduction_of_supported_commands   = false,
    .force_read_attributes_enabled               = false,
    .write_attributes_enabled                    = false,
    .publish_desired_attribute_values_to_mqtt    = false,
    .publish_reported_attribute_values_to_mqtt   = false,
    .node_type     = ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
    .endpoint_type = ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
  };

  unify_dotdot_attribute_store_set_configuration(&test_configuration);
  TEST_ASSERT_EQUAL(1, callback_counter);

  // We invoke even if the configuration is the same:
  unify_dotdot_attribute_store_set_configuration(&test_configuration);
  TEST_ASSERT_EQUAL(2, callback_counter);
}