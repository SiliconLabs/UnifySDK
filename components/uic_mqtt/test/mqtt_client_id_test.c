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
#include "unity.h"
#include "mqtt_client_id.h"

// Generic includes
#include <string.h>

// CMocks
#include "config_mock.h"
#include "datastore_mock.h"

// Test constants
const char *test_client_id_1   = "HelloMQTT";
const char *test_client_id_2   = "por#t€a\%b&l/e(_)r=unti*/m\\eø";
const char expected_string_2[] = "portable_runtime";

const char *test_client_id_3    = "";
static unsigned int string_size = 0;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_mqtt_client_id_test()
{
  // Cannot read the configuration, should also lead to an empty string:
  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_CLIENT_ID,
                                       0,
                                       CONFIG_STATUS_ERROR);
  config_get_as_string_IgnoreArg_result();
  const char *received_string = mqtt_client_id_get_from_config();
  TEST_ASSERT_EQUAL_STRING(test_client_id_3, received_string);

  // Happy case
  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_CLIENT_ID,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_client_id_1);
  received_string = mqtt_client_id_get_from_config();
  TEST_ASSERT_EQUAL_STRING(test_client_id_1, received_string);

  // Check character filters:
  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_CLIENT_ID,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_client_id_2);
  received_string = mqtt_client_id_get_from_config();
  TEST_ASSERT_EQUAL_STRING(expected_string_2, received_string);

  // Empty String
  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_CLIENT_ID,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_client_id_3);
  received_string = mqtt_client_id_get_from_config();
  TEST_ASSERT_EQUAL_STRING(test_client_id_3, received_string);
}

void test_mqtt_client_id_store_to_datastore_not_init()
{
  datastore_is_initialized_ExpectAndReturn(false);
  mqtt_client_store_client_id_to_datastore(test_client_id_1);
}

void test_mqtt_client_id_store_to_datastore_with_matching_data()
{
  string_size = strlen(test_client_id_1) + 1;
  datastore_is_initialized_ExpectAndReturn(true);
  datastore_fetch_arr_ExpectAndReturn(DATASTORE_MQTT_CLIENT_ID_KEY,
                                      NULL,
                                      NULL,
                                      SL_STATUS_OK);
  datastore_fetch_arr_IgnoreArg_value();
  datastore_fetch_arr_IgnoreArg_size();
  datastore_fetch_arr_ReturnArrayThruPtr_value((uint8_t *)test_client_id_1,
                                               string_size);
  datastore_fetch_arr_ReturnThruPtr_size(&string_size);

  datastore_store_arr_ExpectAndReturn(DATASTORE_MQTT_CLIENT_ID_KEY,
                                      NULL,
                                      string_size,
                                      SL_STATUS_OK);
  datastore_store_arr_IgnoreArg_value();
  mqtt_client_store_client_id_to_datastore(test_client_id_1);
}

void test_mqtt_client_id_store_to_datastore_with_not_matching_data()
{
  string_size = strlen(test_client_id_1) + 1;
  datastore_is_initialized_ExpectAndReturn(true);
  datastore_fetch_arr_ExpectAndReturn(DATASTORE_MQTT_CLIENT_ID_KEY,
                                      NULL,
                                      NULL,
                                      SL_STATUS_OK);
  datastore_fetch_arr_IgnoreArg_value();
  datastore_fetch_arr_IgnoreArg_size();
  datastore_fetch_arr_ReturnArrayThruPtr_value((uint8_t *)test_client_id_1,
                                               string_size);
  datastore_fetch_arr_ReturnThruPtr_size(&string_size);

  datastore_store_arr_ExpectAndReturn(DATASTORE_MQTT_CLIENT_ID_KEY,
                                      NULL,
                                      strlen(test_client_id_2) + 1,
                                      SL_STATUS_OK);
  datastore_store_arr_IgnoreArg_value();
  mqtt_client_store_client_id_to_datastore(test_client_id_2);
}