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
#include "mqtt_wrapper.h"

// Static test variables
static void *client_instance = NULL;
static const char *client_id = "test_client_id";

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  TEST_ASSERT_EQUAL(SL_STATUS_OK, mqtt_wrapper_lib_cleanup());
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  TEST_ASSERT_EQUAL(SL_STATUS_OK, mqtt_wrapper_lib_init());
  client_instance = mqtt_wrapper_new(client_id, true, NULL);
  TEST_ASSERT_NOT_NULL(client_instance);
}

void tearDown()
{
  mqtt_wrapper_destroy(client_instance);
}

void test_mqtt_wrapper_mosquitto_set_will_message()
{
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    mqtt_wrapper_set_will_message(client_instance, client_id, 3));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    mqtt_wrapper_set_will_message(client_instance, "", 1));
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER,
                    mqtt_wrapper_set_will_message(NULL, "", 1));
}

void test_mqtt_wrapper_mosquitto_topic_matches_sub()
{
  bool result = false;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    mqtt_wrapper_topic_matches_sub("#", "/test/sub", &result));
  TEST_ASSERT_TRUE(result);

  result = true;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    mqtt_wrapper_topic_matches_sub("/+/+/+", "/test/topic", &result));
  TEST_ASSERT_FALSE(result);

  // Limitation : We cannot compare filters with filters
  result = true;
  TEST_ASSERT_EQUAL(
    SL_STATUS_INVALID_PARAMETER,
    mqtt_wrapper_topic_matches_sub("ucl/by-unid/+/+/Scenes/Commands/AddScene",
                                   "ucl/by-unid/+/+/Scenes/Commands/AddScene",
                                   &result));
  TEST_ASSERT_FALSE(result);

  // Try with invalid parameters
  TEST_ASSERT_EQUAL(
    SL_STATUS_INVALID_PARAMETER,
    mqtt_wrapper_topic_matches_sub(NULL, "/test/topic", &result));

  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER,
                    mqtt_wrapper_topic_matches_sub("/+/#", NULL, &result));
}

void test_mqtt_wrapper_mosquitto_connect()
{
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    mqtt_wrapper_connect(client_instance, "", 1883, 60));
}

void test_mqtt_wrapper_mosquitto_loop_read()
{
  // We have no real broker connection, so these functions by default will just fail.
  TEST_ASSERT_EQUAL(SL_STATUS_NET_MQTT_NO_CONN,
                    mqtt_wrapper_loop_read(client_instance));
}

void test_mqtt_wrapper_mosquitto_loop_write()
{
  // We have no real broker connection, so these functions by default will just fail.
  TEST_ASSERT_EQUAL(SL_STATUS_NET_MQTT_NO_CONN,
                    mqtt_wrapper_loop_write(client_instance));
}

void test_mqtt_wrapper_mosquitto_loop_misc()
{
  // We have no real broker connection, so these functions by default will just fail.
  TEST_ASSERT_EQUAL(SL_STATUS_NET_MQTT_NO_CONN,
                    mqtt_wrapper_loop_misc(client_instance));
}
