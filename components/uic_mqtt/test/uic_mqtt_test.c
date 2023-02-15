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
#include "uic_mqtt.h"
#include "uic_mqtt_int.h"
#include "unity.h"

//Generic includes
#include <stdbool.h>
#include <string.h>

// Contiki test resources
#include "sys/process.h"
#include "contiki_test_helper.h"

// Unify components
#include "sl_log.h"

// CMocks
#include "config_mock.h"
#include "uic_main_externals_mock.h"
#include "mqtt_client_mock.h"
#include "mqtt_client_id_mock.h"

PROCESS_NAME(uic_mqtt_process);

const char *message              = "test";
const char *testhost             = "testhost1";
const int testport               = 1337;
static const char *test_cafile   = "/home/test/cafile";
static const char *test_certfile = "/home/test/certfile";
static const char *test_keyfile  = "/home/test/keyfile";
const char *test_mqtt_client_id  = "test_client_id";

// Connect callbacks variables
static bool after_connect_called     = false;
static bool before_disconnect_called = false;

// This is for making sure the callback-signature stays valid.
static void subscribe_topic_callback(const char *topic,
                                     const char *message,
                                     const size_t message_length)
{}

// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  mqtt_client_mock_Init();
}

// Testing callbacks on disconnect and connect
void after_connect_callback()
{
  after_connect_called = true;
}

void before_disconnect_cabllback()
{
  before_disconnect_called = true;
}

// Tear-down the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  mqtt_client_mock_Destroy();
  return num_failures;
}

void setUp()
{
  contiki_test_helper_init();
  contiki_test_helper_run(0);
}

// Test the initialization and teardown of a Unify MQTT Client process
void test_uic_client_process_init_teardown()
{
  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_HOST,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&testhost);

  config_get_as_int_ExpectAndReturn(CONFIG_KEY_MQTT_PORT, 0, CONFIG_STATUS_OK);
  config_get_as_int_IgnoreArg_result();
  config_get_as_int_ReturnThruPtr_result((int *)&testport);

  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_CAFILE,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_cafile);

  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_CERTFILE,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_certfile);

  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_KEYFILE,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_keyfile);

  mqtt_client_id_get_from_config_ExpectAndReturn(test_mqtt_client_id);

  mqtt_client_event_Expect(NULL, MQTT_EVENT_CONNECT);
  mqtt_client_file_descriptor_IgnoreAndReturn(0);
  mqtt_client_new_ExpectAndReturn(testhost,
                                  testport,
                                  NULL,
                                  test_cafile,
                                  test_certfile,
                                  test_keyfile,
                                  uic_mqtt_process_post,
                                  uic_mqtt_process_post_delayed,
                                  uic_mqtt_process_count_events,
                                  NULL);
  mqtt_client_new_IgnoreArg_client_id();
  mqtt_client_setup_ExpectAndReturn(NULL, true);
  mqtt_client_disconnect_Expect(NULL);
  mqtt_client_on_connect_callback_set_Expect(NULL, uic_mqtt_process_on_connect);
  mqtt_client_on_disconnect_callback_set_Expect(NULL,
                                                uic_mqtt_process_on_disconnect);
  uic_main_ext_register_rfd_IgnoreAndReturn(SL_STATUS_OK);

  uic_mqtt_setup();
  contiki_test_helper_run(0);

  process_exit(&uic_mqtt_process);
  contiki_test_helper_run(0);

  // Testing init and teardown callbacks before disconnect and after connecting
  sl_log_debug("Test", "after_connect_called: %d", after_connect_called);
  sl_log_debug("Test",
               "before_disconnect_called: %d",
               before_disconnect_called);
}

// Test the initialization and teardown of a Unify MQTT Client process
void test_uic_client_process_init_teardown_no_client_id()
{
  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_HOST,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&testhost);

  config_get_as_int_ExpectAndReturn(CONFIG_KEY_MQTT_PORT, 0, CONFIG_STATUS_OK);
  config_get_as_int_IgnoreArg_result();
  config_get_as_int_ReturnThruPtr_result((int *)&testport);

  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_CAFILE,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_cafile);

  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_CERTFILE,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_certfile);

  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_KEYFILE,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_keyfile);

  mqtt_client_id_get_from_config_ExpectAndReturn(test_mqtt_client_id);

  mqtt_client_after_connect_callback_set_Expect(NULL, after_connect_callback);
  mqtt_client_before_disconnect_callback_set_Expect(
    NULL,
    before_disconnect_cabllback);
  mqtt_client_event_Expect(NULL, MQTT_EVENT_CONNECT);
  mqtt_client_file_descriptor_IgnoreAndReturn(0);
  mqtt_client_new_ExpectAndReturn(testhost,
                                  testport,
                                  NULL,
                                  test_cafile,
                                  test_certfile,
                                  test_keyfile,
                                  uic_mqtt_process_post,
                                  uic_mqtt_process_post_delayed,
                                  uic_mqtt_process_count_events,
                                  NULL);
  mqtt_client_new_IgnoreArg_client_id();

  mqtt_client_setup_ExpectAndReturn(NULL, true);
  mqtt_client_disconnect_Expect(NULL);
  mqtt_client_on_connect_callback_set_Expect(NULL, uic_mqtt_process_on_connect);
  mqtt_client_on_disconnect_callback_set_Expect(NULL,
                                                uic_mqtt_process_on_disconnect);
  uic_main_ext_register_rfd_IgnoreAndReturn(SL_STATUS_OK);
  uic_mqtt_set_after_connect_callback(after_connect_callback);
  uic_mqtt_set_before_disconnect_callback(before_disconnect_cabllback);

  uic_mqtt_setup();
  contiki_test_helper_run(0);

  //mqtt_client_disconnect_Expect(NULL);
  process_exit(&uic_mqtt_process);
  contiki_test_helper_run(0);
}

// Make sure uic_mqtt_setup() does its business of fetching configuration, trying
// to connect to a broker.
void test_uic_client_setup_happy()
{
  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_HOST,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&testhost);
  config_get_as_int_ExpectAndReturn(CONFIG_KEY_MQTT_PORT, 0, CONFIG_STATUS_OK);
  config_get_as_int_IgnoreArg_result();
  config_get_as_int_ReturnThruPtr_result((int *)&testport);

  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_CAFILE,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_cafile);

  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_CERTFILE,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_certfile);

  config_get_as_string_ExpectAndReturn(CONFIG_KEY_MQTT_KEYFILE,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_keyfile);

  mqtt_client_id_get_from_config_ExpectAndReturn(test_mqtt_client_id);

  mqtt_client_new_ExpectAndReturn(testhost,
                                  testport,
                                  NULL,
                                  test_cafile,
                                  test_certfile,
                                  test_keyfile,
                                  uic_mqtt_process_post,
                                  uic_mqtt_process_post_delayed,
                                  uic_mqtt_process_count_events,
                                  NULL);
  mqtt_client_new_IgnoreArg_client_id();

  mqtt_client_event_Expect(NULL, MQTT_EVENT_CONNECT);
  mqtt_client_file_descriptor_IgnoreAndReturn(0);
  mqtt_client_on_connect_callback_set_Expect(NULL, uic_mqtt_process_on_connect);
  mqtt_client_on_disconnect_callback_set_Expect(NULL,
                                                uic_mqtt_process_on_disconnect);
  mqtt_client_setup_ExpectAndReturn(NULL, true);
  uic_mqtt_setup();
  contiki_test_helper_run(0);

  mqtt_client_disconnect_Expect(NULL);
  process_exit(&uic_mqtt_process);
  contiki_test_helper_run(0);
}

// Ensure API correctness.
void test_uic_client_publish_happy()
{
  mqtt_client_publish_Expect(NULL,
                             "UIC/Client/test",
                             message,
                             strlen(message),
                             false);
  uic_mqtt_publish("UIC/Client/test", message, strlen(message), false);
}

// Ensure API correctness.
void test_uic_client_subscribe_happy()
{
  mqtt_client_subscribe_Expect(NULL,
                               "UIC/Client/test",
                               subscribe_topic_callback);
  uic_mqtt_subscribe("UIC/Client/test", subscribe_topic_callback);
}

// Ensure API correctness.
void test_uic_client_unsubscribe_happy()
{
  mqtt_client_unsubscribe_Expect(NULL, "UIC/Client/test", NULL);
  uic_mqtt_unsubscribe("UIC/Client/test", NULL);
}
