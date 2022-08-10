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
// Standard library
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// Testing+mocking framework
#include "sl_status.h"
#include "unity.h"
#include "cmock.h"
#include "unity_internals.h"

// Unify components
#include "sl_log.h"

extern "C" {

// This component
#include "mqtt_client.h"
#include "mqtt_wrapper_mock.h"

const char *testhost      = "testhost";
const int testport        = 1337;
const char *test_cafile   = "/test/cafile";
const char *test_certfile = "/test/certfile";
const char *test_keyfile  = "/test/keyfile";

static char test_id[16];
static bool single_subscription_received          = false;
static bool wildcard_subsciption_message_received = false;
static bool multi_subscription_received           = false;
const char *testmessage                           = "This is a test-message.";
static int sub_callback_count                     = 0;
static int wildcard_sub_count                     = 0;
static bool event_loop_enabled                    = true;
static int on_cb_count                            = 0;
static bool callback1_called                      = false;
static bool callback2_called                      = false;

void *connection_instance_mock;
mqtt_client_t client_instance;

#define LOG_TAG "mqtt_client_test"

int event_sender(const int event, void *procdata)
{
  if (event_loop_enabled) {
    sl_log_debug(LOG_TAG, "Passing event: %s\n", event_names[event]);
    mqtt_client_event(client_instance, event);
  } else {
    sl_log_debug(LOG_TAG, "Dropping event: %s\n", event_names[event]);
  }
  return 0;
}

void delayed_event_sender(const int event, const unsigned long milliseconds)
{
  if (event_loop_enabled) {
    sl_log_debug(LOG_TAG,
                 "Passing (not-so) delayed event: %s (%ld ms)\n",
                 event_names[event],
                 milliseconds);
    mqtt_client_event(client_instance, event);
  } else {
    sl_log_debug(LOG_TAG, "Dropping delayed event: %s\n", event_names[event]);
  }
}

int event_counter(const int event, void *procdata)
{
  return 0;
}

void (*mqtt_client_test_on_connect)(void *inst, void *obj, int result_code);
void (*mqtt_client_test_on_disconnect)(void *inst, void *obj, int result_code);
void (*mqtt_client_test_on_message)(void *inst,
                                    void *obj,
                                    const struct mqtt_message *message);

void connect_callback_set_stub(void *instance,
                               void (*on_connect)(void *inst,
                                                  void *obj,
                                                  int result_code),
                               int cmock_num_calls)
{
  sl_log_debug(LOG_TAG, "On-connect callback set stub: %d\n", cmock_num_calls);
  mqtt_client_test_on_connect = on_connect;
}

void disconnect_callback_set_stub(void *instance,
                                  void (*on_disconnect)(void *inst,
                                                        void *obj,
                                                        int result_code),
                                  int cmock_num_calls)
{
  sl_log_debug(LOG_TAG,
               "On-disconnect callback set stub: %d\n",
               cmock_num_calls);
  mqtt_client_test_on_disconnect = on_disconnect;
}

void message_callback_set_stub(
  void *instance,
  void (*on_message)(void *inst, void *obj, const struct mqtt_message *message),
  int cmock_num_calls)
{
  sl_log_debug(LOG_TAG, "On-message callback set stub: %d\n", cmock_num_calls);
  mqtt_client_test_on_message = on_message;
}

unsigned int mqtt_wrapper_topic_matches_sub_stub(const char *sub,
                                                 const char *topic,
                                                 bool *result,
                                                 int cmock_num_calls)
{
  *result = true;
  return SL_STATUS_OK;
}
void subscription_callback(const char *topic,
                           const char *message,
                           const size_t message_length)
{
  sl_log_debug(LOG_TAG, "Subscriber got a message: %s: %s\n", topic, message);
  single_subscription_received = true;
  sub_callback_count++;
}

void subscription_callback_multi(const char *topic,
                                 const char *message,
                                 const size_t message_length)
{
  sl_log_debug(LOG_TAG, "Subscriber got a message: %s: %s\n", topic, message);
  multi_subscription_received = true;
  sub_callback_count++;
}

void subscription_callback_wildcard(const char *topic,
                                    const char *message,
                                    const size_t message_length)
{
  sl_log_debug(LOG_TAG, "Subscriber got a message: %s: %s\n", topic, message);
  wildcard_subsciption_message_received = true;
  wildcard_sub_count++;
}

void subscription_callback_wildcard_second(const char *topic,
                                           const char *message,
                                           const size_t message_length)
{
  sl_log_debug(LOG_TAG,
               "Subscriber (second) got a message: %s: %s\n",
               topic,
               message);
  wildcard_subsciption_message_received = true;
  wildcard_sub_count++;
}

void callback_test1(const char *topic,
                    const char *message,
                    const size_t message_length)
{
  sl_log_debug(LOG_TAG, "Subscriber got a message: %s: %s\n", topic, message);
  callback1_called = true;
  on_cb_count++;
}

void callback_test2(const char *topic,
                    const char *message,
                    const size_t message_length)
{
  sl_log_debug(LOG_TAG, "Subscriber got a message: %s: %s\n", topic, message);
  callback2_called = true;
  on_cb_count++;
}

// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  connection_instance_mock = malloc(1);
  snprintf(test_id, sizeof(test_id), "uic_%i", getpid());
  mqtt_wrapper_mock_Init();
}

// Tear-down the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  mqtt_wrapper_mock_Destroy();
  free(connection_instance_mock);
  return num_failures;
}

void setUp()
{
  single_subscription_received          = false;
  wildcard_subsciption_message_received = false;
  multi_subscription_received           = false;
  callback1_called                      = false;
  callback2_called                      = false;
  event_loop_enabled                    = true;
}

void test_mqtt_client_init_teardown_happy()
{
  mqtt_wrapper_lib_init_ExpectAndReturn(0);
  mqtt_wrapper_new_ExpectAndReturn(test_id, true, NULL, NULL);
  mqtt_wrapper_new_IgnoreArg_obj();
  mqtt_wrapper_destroy_Expect(NULL);
  mqtt_wrapper_lib_cleanup_ExpectAndReturn(0);
  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);
  mqtt_client_setup(client_instance);
  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

static void simple_connect()
{
  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,          // (client)id
                                     true,             // clean_session
                                     client_instance,  // obj
                                     connection_instance_mock);  // return
    mqtt_wrapper_new_IgnoreArg_obj();
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_connect_ExpectAndReturn(connection_instance_mock,  // instance
                                         testhost,                  // host
                                         testport,                  // port
                                         0,              // keepalive.
                                         SL_STATUS_OK);  // return
    mqtt_wrapper_connect_IgnoreArg_keepalive();

    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(0);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);

  {  // Mock section
    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
  }  // End of mock section

  mqtt_client_setup(client_instance);
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.
}

static void simple_disconnect()
{
  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

static void simple_publish(const char *topic)
{
  mqtt_wrapper_publish_ExpectAndReturn(connection_instance_mock,
                                       NULL,
                                       topic,
                                       strlen(testmessage),
                                       testmessage,
                                       MQTT_CLIENT_QoS,
                                       true,
                                       SL_STATUS_OK);
  mqtt_client_publish(client_instance,
                      topic,
                      testmessage,
                      strlen(testmessage),
                      true);
}

void test_mqtt_client_publish_happy()
{
  simple_connect();
  simple_publish("MQTT/Client/test");
  simple_disconnect();
}

void expect_unretain(const char *topic)
{
  mqtt_wrapper_publish_ExpectAndReturn(connection_instance_mock,
                                       NULL,
                                       topic,
                                       0,
                                       0,
                                       MQTT_CLIENT_QoS,
                                       true,
                                       SL_STATUS_OK);
  mqtt_wrapper_publish_IgnoreArg_payload();
}

void test_mqtt_client_unretain()
{
  simple_connect();

  simple_publish("TopicA/Sub1");
  simple_publish("TopicA/Sub2");
  simple_publish("TopicB/Sub1");

  TEST_ASSERT_EQUAL(3, mqtt_client_count_topics(client_instance, "Topic"));
  TEST_ASSERT_EQUAL(2, mqtt_client_count_topics(client_instance, "TopicA"));
  TEST_ASSERT_EQUAL(1, mqtt_client_count_topics(client_instance, "TopicB"));

  TEST_ASSERT_EQUAL(1,
                    mqtt_client_count_topics(client_instance, "TopicB/Sub1"));
  TEST_ASSERT_EQUAL(1,
                    mqtt_client_count_topics(client_instance, "TopicB/Sub1"));

  expect_unretain("TopicA/Sub1");
  expect_unretain("TopicA/Sub2");

  mqtt_client_unretain(client_instance, "TopicA");
  TEST_ASSERT_EQUAL(1, mqtt_client_count_topics(client_instance, "Topic"));

  //Try to unretain something that does not exist
  mqtt_client_unretain(client_instance, "Floppic");

  //Try to unretain TopicA again
  mqtt_client_unretain(client_instance, "TopicA");

  simple_disconnect();
}

void test_mqtt_client_unretain_by_regex()
{
  simple_connect();

  simple_publish("TopicA/Sub1");
  simple_publish("TopicA/Sub2");
  simple_publish("TopicB/Sub1");

  expect_unretain("TopicA/Sub1");
  expect_unretain("TopicA/Sub2");

  mqtt_client_unretain_by_regex(client_instance, ".*A\\/Sub.*");

  simple_disconnect();
}

void test_mqtt_client_subscribe_single_callback_happy()
{
  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,  // (client)id
                                     true,     // clean_session
                                     NULL,     // obj
                                     connection_instance_mock);  // return
    mqtt_wrapper_new_IgnoreArg_obj();

    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_connect_ExpectAndReturn(connection_instance_mock,  // instance
                                         testhost,                  // host
                                         testport,                  // port
                                         0,              // keepalive.
                                         SL_STATUS_OK);  // return
    mqtt_wrapper_connect_IgnoreArg_keepalive();

    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(0);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);

  // Mock section
  {
    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
    mqtt_wrapper_topic_matches_sub_Stub(mqtt_wrapper_topic_matches_sub_stub);
  }
  // End of mock section

  mqtt_client_setup(client_instance);
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  // Mock section
  {
    mqtt_wrapper_subscribe_ExpectAndReturn(connection_instance_mock,
                                           NULL,
                                           "MQTT/Client/test",
                                           MQTT_CLIENT_QoS,
                                           SL_STATUS_OK);
  }
  // End of mock section

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.
  mqtt_client_subscribe(client_instance,
                        "MQTT/Client/test",
                        subscription_callback);

  struct mqtt_message *mock_message
    = (struct mqtt_message *)malloc(sizeof(struct mqtt_message));
  mock_message->topic      = "MQTT/Client/test";
  mock_message->payload    = (void *)testmessage;
  mock_message->payloadlen = strlen(testmessage);
  mock_message->qos        = 0;
  mock_message->retain     = false;

  mqtt_client_test_on_message(NULL, client_instance, mock_message);
  free(mock_message);
  TEST_ASSERT_TRUE_MESSAGE(
    single_subscription_received,
    "single_subscription_received flag was not flipped.");
  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

void test_mqtt_client_subscribe_multiple_rejected()
{
  sub_callback_count = 0;
  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,  // (client)id
                                     true,     // clean_session
                                     NULL,     // obj
                                     connection_instance_mock);  // return
    mqtt_wrapper_new_IgnoreArg_obj();

    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_connect_ExpectAndReturn(connection_instance_mock,  // instance
                                         testhost,                  // host
                                         testport,                  // port
                                         0,              // keepalive.
                                         SL_STATUS_OK);  // return
    mqtt_wrapper_connect_IgnoreArg_keepalive();

    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(0);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);

  // Mock section
  {
    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
    mqtt_wrapper_topic_matches_sub_Stub(mqtt_wrapper_topic_matches_sub_stub);
  }
  // End of mock section

  mqtt_client_setup(client_instance);
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  // Mock section
  {
    mqtt_wrapper_subscribe_ExpectAndReturn(connection_instance_mock,
                                           NULL,
                                           "MQTT/Client/test",
                                           MQTT_CLIENT_QoS,
                                           SL_STATUS_OK);
  }
  // End of mock section

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.
  mqtt_client_subscribe(client_instance,
                        "MQTT/Client/test",
                        subscription_callback);
  mqtt_client_subscribe(client_instance,
                        "MQTT/Client/test",
                        subscription_callback);

  struct mqtt_message *mock_message
    = (struct mqtt_message *)malloc(sizeof(struct mqtt_message));
  mock_message->topic      = "MQTT/Client/test";
  mock_message->payload    = (void *)testmessage;
  mock_message->payloadlen = strlen(testmessage);
  mock_message->qos        = 0;
  mock_message->retain     = false;

  mqtt_client_test_on_message(NULL, client_instance, mock_message);
  free(mock_message);
  TEST_ASSERT_EQUAL_INT(1, sub_callback_count);
  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

void test_mqtt_client_subscribe_multiple_callback_happy()
{
  sub_callback_count = 0;

  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,  // (client)id
                                     true,     // clean_session
                                     NULL,     // obj
                                     connection_instance_mock);  // return
    mqtt_wrapper_new_IgnoreArg_obj();

    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_connect_ExpectAndReturn(connection_instance_mock,  // instance
                                         testhost,                  // host
                                         testport,                  // port
                                         0,              // keepalive.
                                         SL_STATUS_OK);  // return
    mqtt_wrapper_connect_IgnoreArg_keepalive();

    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(0);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);

  // Mock section
  {
    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
    mqtt_wrapper_topic_matches_sub_Stub(mqtt_wrapper_topic_matches_sub_stub);
  }
  // End of mock section

  mqtt_client_setup(client_instance);
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  // Mock section for first callback
  {
    mqtt_wrapper_subscribe_ExpectAndReturn(connection_instance_mock,
                                           NULL,
                                           "UIC/MQTT/Client/test",
                                           MQTT_CLIENT_QoS,
                                           SL_STATUS_OK);
  }
  // End of mock section

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.
  mqtt_client_subscribe(client_instance,
                        "UIC/MQTT/Client/test",
                        subscription_callback);

  // The second subscribtion with the same topic shall not trigger
  // second mqtt_wrapper_subsribe call, rather, it will be added in
  // the internal routing.
  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.

  mqtt_client_subscribe(client_instance,
                        "UIC/MQTT/Client/test",
                        subscription_callback_multi);

  struct mqtt_message *mock_message
    = (struct mqtt_message *)malloc(sizeof(struct mqtt_message));
  mock_message->topic      = "UIC/MQTT/Client/test";
  mock_message->payload    = (void *)testmessage;
  mock_message->payloadlen = strlen(testmessage);
  mock_message->qos        = 0;
  mock_message->retain     = false;

  mqtt_client_test_on_message(NULL, client_instance, mock_message);
  free(mock_message);
  // check that the two registered functions are called
  TEST_ASSERT_EQUAL_INT(2, sub_callback_count);
  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

void test_mqtt_client_subscribe_wildcard_happy()
{
  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,  // (client)id
                                     true,     // clean_session
                                     NULL,     // obj
                                     connection_instance_mock);  // return
    mqtt_wrapper_new_IgnoreArg_obj();

    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_connect_ExpectAndReturn(connection_instance_mock,  // instance
                                         testhost,                  // host
                                         testport,                  // port
                                         0,              // keepalive.
                                         SL_STATUS_OK);  // return
    mqtt_wrapper_connect_IgnoreArg_keepalive();

    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(0);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);

  // Mock section
  {
    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
    mqtt_wrapper_topic_matches_sub_Stub(mqtt_wrapper_topic_matches_sub_stub);
  }
  // End of mock section

  mqtt_client_setup(client_instance);
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  // Mock section for Multi Level Wildcard: #
  {
    mqtt_wrapper_subscribe_ExpectAndReturn(connection_instance_mock,
                                           NULL,
                                           "MQTT/Client/#",
                                           MQTT_CLIENT_QoS,
                                           SL_STATUS_OK);
  }
  // End of mock section

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.
  mqtt_client_subscribe(client_instance,
                        "MQTT/Client/#",
                        subscription_callback_wildcard);

 //  Mock section for Single Level Wildcard: +
  {
    mqtt_wrapper_subscribe_ExpectAndReturn(connection_instance_mock,
                                           NULL,
                                           "MQTT/+/test",
                                           MQTT_CLIENT_QoS,
                                           SL_STATUS_OK);
  }
  // End of mock section

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.
  mqtt_client_subscribe(client_instance,
                        "MQTT/+/test",
                        subscription_callback_wildcard_second);

  struct mqtt_message *mock_message
    = (struct mqtt_message *)malloc(sizeof(struct mqtt_message));
  mock_message->topic      = "MQTT/Client/test";
  mock_message->payload    = (void *)testmessage;
  mock_message->payloadlen = strlen(testmessage);
  mock_message->qos        = 0;
  mock_message->retain     = false;

  mqtt_client_test_on_message(NULL, client_instance, mock_message);
  free(mock_message);
  TEST_ASSERT_TRUE_MESSAGE(
    wildcard_subsciption_message_received,
    "wildcard_subscription_message_received flag was not flipped.");
  TEST_ASSERT_EQUAL_INT(2, wildcard_sub_count);
  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

void test_mqtt_client_unsubscribe_callback_happy()
{
  sub_callback_count = 0;
  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,  // (client)id
                                     true,     // clean_session
                                     NULL,     // obj
                                     connection_instance_mock);  // return
    mqtt_wrapper_new_IgnoreArg_obj();

    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_connect_ExpectAndReturn(connection_instance_mock,  // instance
                                         testhost,                  // host
                                         testport,                  // port
                                         0,              // keepalive.
                                         SL_STATUS_OK);  // return
    mqtt_wrapper_connect_IgnoreArg_keepalive();

    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(0);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);

  // Mock section
  {
    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
    mqtt_wrapper_topic_matches_sub_Stub(mqtt_wrapper_topic_matches_sub_stub);
  }
  // End of mock section

  mqtt_client_setup(client_instance);
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  // Mock section for first callback
  {
    mqtt_wrapper_subscribe_ExpectAndReturn(connection_instance_mock,
                                           NULL,
                                           "UIC/MQTT/Client/test",
                                           MQTT_CLIENT_QoS,
                                           SL_STATUS_OK);
  }
  // End of mock section

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.
  mqtt_client_subscribe(client_instance,
                        "UIC/MQTT/Client/test",
                        subscription_callback);

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.

  mqtt_client_subscribe(client_instance,
                        "UIC/MQTT/Client/test",
                        subscription_callback_multi);

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.

  mqtt_client_unsubscribe(client_instance,
                          "UIC/MQTT/Client/test",
                          subscription_callback_multi);

  struct mqtt_message *mock_message
    = (struct mqtt_message *)malloc(sizeof(struct mqtt_message));
  mock_message->topic      = "UIC/MQTT/Client/test";
  mock_message->payload    = (void *)testmessage;
  mock_message->payloadlen = strlen(testmessage);
  mock_message->qos        = 0;
  mock_message->retain     = false;

  mqtt_client_test_on_message(NULL, client_instance, mock_message);
  free(mock_message);
  // check that the only one of the registered function are called
  TEST_ASSERT_EQUAL_INT(1, sub_callback_count);
  TEST_ASSERT_TRUE_MESSAGE(
    single_subscription_received,
    "single_subscription_received flag was not flipped.");
  TEST_ASSERT_FALSE_MESSAGE(multi_subscription_received,
                            "multi_subscription_received flag was flipped.");
  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

void test_mqtt_client_publish_flushed_on_connect()
{
  // Messages that are published before the client connects to a broker should be
  // queued up internally and then pushed over to the broker as soon a connection
  // has been established.

  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,
                                     true,
                                     client_instance,
                                     connection_instance_mock);
    mqtt_wrapper_new_IgnoreArg_obj();

    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);
  mqtt_client_setup(client_instance);

  // We'll publish the message twice to make sure we're not just flushing out
  // the first one and stopping after that.
  {  // Mock section
    mqtt_wrapper_publish_ExpectAndReturn(connection_instance_mock,
                                         NULL,
                                         "MQTT/Client/test",
                                         strlen(testmessage),
                                         testmessage,
                                         MQTT_CLIENT_QoS,
                                         false,
                                         SL_STATUS_OK);
    mqtt_wrapper_publish_ExpectAndReturn(connection_instance_mock,
                                         NULL,
                                         "MQTT/Client/test",
                                         strlen(testmessage),
                                         testmessage,
                                         MQTT_CLIENT_QoS,
                                         false,
                                         SL_STATUS_OK);
  }  // End of mock section

  mqtt_client_publish(client_instance,
                      "MQTT/Client/test",
                      testmessage,
                      strlen(testmessage),
                      false);
  mqtt_client_publish(client_instance,
                      "MQTT/Client/test",
                      testmessage,
                      strlen(testmessage),
                      false);

  {  // Mock section
    mqtt_wrapper_connect_ExpectAndReturn(
      connection_instance_mock,
      testhost,
      testport,
      MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS / 1000,
      SL_STATUS_OK);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
  }  // End of mock section
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  {  // Mock section
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
  }  // End of mock section
  // Kick the callback and tell the client it's connected.
  mqtt_client_test_on_connect(NULL, client_instance, 0);

  {  // Mock section
    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(SL_STATUS_OK);
  }  // End of mock section

  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

void test_mqtt_client_publish_flushed_on_disconnect()
{
  // Outgoing messages that have been queued up should be pushed to the broker
  // in case the client is about to do a graceful disconnect.

  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,
                                     true,
                                     client_instance,
                                     connection_instance_mock);
    mqtt_wrapper_new_IgnoreArg_obj();

    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);
  mqtt_client_setup(client_instance);

  {  // Mock section
    mqtt_wrapper_connect_ExpectAndReturn(
      connection_instance_mock,
      testhost,
      testport,
      MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS / 1000,
      SL_STATUS_OK);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
  }  // End of mock section
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  {  // Mock section
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
  }  // End of mock section
  // Kick the callback and tell the client it's connected.
  mqtt_client_test_on_connect(NULL, client_instance, 0);

  // We'll publish the message twice to make sure we're not just flushing out
  // the first one and stopping after that.
  {  // Mock section
    mqtt_wrapper_publish_ExpectAndReturn(connection_instance_mock,
                                         NULL,
                                         "MQTT/Client/test",
                                         strlen(testmessage),
                                         testmessage,
                                         MQTT_CLIENT_QoS,
                                         false,
                                         SL_STATUS_OK);
    mqtt_wrapper_publish_ExpectAndReturn(connection_instance_mock,
                                         NULL,
                                         "MQTT/Client/test",
                                         strlen(testmessage),
                                         testmessage,
                                         MQTT_CLIENT_QoS,
                                         false,
                                         SL_STATUS_OK);
  }  // End of mock section

  event_loop_enabled = false;

  mqtt_client_publish(client_instance,
                      "MQTT/Client/test",
                      testmessage,
                      strlen(testmessage),
                      false);
  mqtt_client_publish(client_instance,
                      "MQTT/Client/test",
                      testmessage,
                      strlen(testmessage),
                      false);

  {  // Mock section
    mqtt_wrapper_disconnect_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(SL_STATUS_OK);
  }  // End of mock section

  mqtt_client_disconnect(client_instance);

  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

void test_mqtt_client_retry_connect()
{
  // Make sure that if connecting to a broker fails, that the client will retry
  // until successful.

  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,
                                     true,
                                     client_instance,
                                     connection_instance_mock);
    mqtt_wrapper_new_IgnoreArg_obj();

    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);
  mqtt_client_setup(client_instance);

  {  // Mock section
    errno = ECONNREFUSED;
    mqtt_wrapper_connect_ExpectAndReturn(
      connection_instance_mock,
      testhost,
      testport,
      MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS / 1000,
      SL_STATUS_ERRNO);
    mqtt_wrapper_connect_ExpectAndReturn(
      connection_instance_mock,
      testhost,
      testport,
      MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS / 1000,
      SL_STATUS_OK);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
  }  // End of mock section
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  {  // Mock section
    mqtt_wrapper_disconnect_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(SL_STATUS_OK);
  }  // End of mock section

  mqtt_client_disconnect(client_instance);

  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

void test_mqtt_client_reconnect_on_dropped_connection()
{
  // If the connection is unexpectedly dropped, the client should try to reconnect.

  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,
                                     true,
                                     client_instance,
                                     connection_instance_mock);
    mqtt_wrapper_new_IgnoreArg_obj();

    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);
  mqtt_client_setup(client_instance);

  {  // Mock section
    mqtt_wrapper_connect_ExpectAndReturn(
      connection_instance_mock,
      testhost,
      testport,
      MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS / 1000,
      SL_STATUS_OK);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
  }  // End of mock section
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  {  // Mock section
    mqtt_wrapper_connect_ExpectAndReturn(
      connection_instance_mock,
      testhost,
      testport,
      MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS / 1000,
      SL_STATUS_OK);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
  }  // End of mock section
  mqtt_client_test_on_disconnect(connection_instance_mock, client_instance, 1);

  {  // Mock section
    mqtt_wrapper_disconnect_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(SL_STATUS_OK);
  }  // End of mock section

  mqtt_client_disconnect(client_instance);

  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

void test_mqtt_client_resubscribe_on_dropped_connection()
{
  // If the connection is unexpectedly dropped, the client should try to reconnect.

  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,
                                     true,
                                     client_instance,
                                     connection_instance_mock);
    mqtt_wrapper_new_IgnoreArg_obj();

    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);
  mqtt_client_setup(client_instance);

  {  // Mock section
    mqtt_wrapper_connect_ExpectAndReturn(
      connection_instance_mock,
      testhost,
      testport,
      MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS / 1000,
      SL_STATUS_OK);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
  }  // End of mock section
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  {  // Mock section
    mqtt_wrapper_connect_ExpectAndReturn(
      connection_instance_mock,
      testhost,
      testport,
      MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS / 1000,
      SL_STATUS_OK);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
  }  // End of mock section

  {  // Mock section
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
  }  // End of mock section

  // Kick the callback and tell the client it's connected.
  mqtt_client_test_on_connect(NULL, client_instance, 0);

  {  // Mock section
    mqtt_wrapper_subscribe_ExpectAndReturn(connection_instance_mock,
                                           NULL,
                                           "MQTT/Client/test",
                                           MQTT_CLIENT_QoS,
                                           SL_STATUS_OK);
  }  // End of mock section

  mqtt_client_subscribe(client_instance,
                        "MQTT/Client/test",
                        subscription_callback);

  // Mock dropping a connection
  mqtt_client_test_on_disconnect(connection_instance_mock, client_instance, 1);

  {  // Mock section
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);

    // This is the crucial thing: Subscribe has to be called again when we
    // have reconnected.
    mqtt_wrapper_subscribe_ExpectAndReturn(connection_instance_mock,
                                           NULL,
                                           "MQTT/Client/test",
                                           MQTT_CLIENT_QoS,
                                           SL_STATUS_OK);
  }  // End of mock section

  // Kick the callback again and tell the client it's re-connected. Now we should
  // get the subscribe calls when the client tries to re-subscribe.
  mqtt_client_test_on_connect(NULL, client_instance, 0);

  {  // Mock section
    mqtt_wrapper_disconnect_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(SL_STATUS_OK);
  }  // End of mock section

  mqtt_client_disconnect(client_instance);

  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}

static void simple_connect_with_tls_certificate()
{
  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,          // (client)id
                                     true,             // clean_session
                                     client_instance,  // obj
                                     connection_instance_mock);  // return
    mqtt_wrapper_new_IgnoreArg_obj();
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_connect_ExpectAndReturn(connection_instance_mock,  // instance
                                         testhost,                  // host
                                         testport,                  // port
                                         0,              // keepalive.
                                         SL_STATUS_OK);  // return
    mqtt_wrapper_connect_IgnoreArg_keepalive();

    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(0);
    mqtt_wrapper_tls_set_ExpectAndReturn(connection_instance_mock,
                                         test_cafile,
                                         test_certfile,
                                         test_keyfile,
                                         SL_STATUS_OK);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    test_cafile,
                                    test_certfile,
                                    test_keyfile,
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);

  {  // Mock section
    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
  }  // End of mock section

  mqtt_client_setup(client_instance);
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.
}

void test_mqtt_client_tls_cert_publish_happy()
{
  simple_connect_with_tls_certificate();
  simple_publish("MQTT/Client/test");
  simple_disconnect();
}

void test_simple_connect_with_tls_certificate_fail()
{
  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,          // (client)id
                                     true,             // clean_session
                                     client_instance,  // obj
                                     connection_instance_mock);  // return
    mqtt_wrapper_new_IgnoreArg_obj();
    mqtt_wrapper_tls_set_ExpectAndReturn(connection_instance_mock,
                                         test_cafile,
                                         test_certfile,
                                         test_keyfile,
                                         SL_STATUS_FAIL);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    test_cafile,
                                    test_certfile,
                                    test_keyfile,
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);

  TEST_ASSERT_EQUAL(mqtt_client_setup(client_instance), SL_STATUS_FAIL);

  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,          // (client)id
                                     true,             // clean_session
                                     client_instance,  // obj
                                     connection_instance_mock);  // return
    mqtt_wrapper_new_IgnoreArg_obj();
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    test_cafile,
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);

  TEST_ASSERT_EQUAL(mqtt_client_setup(client_instance), SL_STATUS_FAIL);
}

void test_mqtt_client_subscribe_multiple_callback_with_wildcard()
{
  on_cb_count = 0;

  {  // Mock section
    mqtt_wrapper_lib_init_ExpectAndReturn(0);
    mqtt_wrapper_new_ExpectAndReturn(test_id,  // (client)id
                                     true,     // clean_session
                                     NULL,     // obj
                                     connection_instance_mock);  // return
    mqtt_wrapper_new_IgnoreArg_obj();

    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_socket_ExpectAndReturn(connection_instance_mock, 0);
    mqtt_wrapper_loop_read_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_loop_write_ExpectAndReturn(connection_instance_mock,
                                            SL_STATUS_OK);
    mqtt_wrapper_loop_misc_ExpectAndReturn(connection_instance_mock,
                                           SL_STATUS_OK);
    mqtt_wrapper_connect_ExpectAndReturn(connection_instance_mock,  // instance
                                         testhost,                  // host
                                         testport,                  // port
                                         0,              // keepalive.
                                         SL_STATUS_OK);  // return
    mqtt_wrapper_connect_IgnoreArg_keepalive();

    mqtt_wrapper_destroy_Expect(connection_instance_mock);
    mqtt_wrapper_lib_cleanup_ExpectAndReturn(0);
  }  // End of mock section

  client_instance = mqtt_client_new(testhost,
                                    testport,
                                    test_id,
                                    "",
                                    "",
                                    "",
                                    event_sender,
                                    delayed_event_sender,
                                    event_counter);

  // Mock section
  {
    mqtt_wrapper_connect_callback_set_Stub(connect_callback_set_stub);
    mqtt_wrapper_disconnect_callback_set_Stub(disconnect_callback_set_stub);
    mqtt_wrapper_message_callback_set_Stub(message_callback_set_stub);
    mqtt_wrapper_topic_matches_sub_Stub(mqtt_wrapper_topic_matches_sub_stub);
  }
  // End of mock section

  mqtt_client_setup(client_instance);
  mqtt_client_event(client_instance, MQTT_EVENT_CONNECT);

  // Mock section for first callback
  {
    mqtt_wrapper_subscribe_ExpectAndReturn(connection_instance_mock,
                                           NULL,
                                           "Topic1/#",
                                           MQTT_CLIENT_QoS,
                                           SL_STATUS_OK);
  }

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.

  mqtt_client_subscribe(client_instance, "Topic1/#", callback_test1);

  // Mock section for second callback
  {
    mqtt_wrapper_subscribe_ExpectAndReturn(connection_instance_mock,
                                           NULL,
                                           "Topic1/item1",
                                           MQTT_CLIENT_QoS,
                                           SL_STATUS_OK);
  }

  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.

  mqtt_client_subscribe(client_instance, "Topic1/item1", callback_test2);

  struct mqtt_message *mock_message
    = (struct mqtt_message *)malloc(sizeof(struct mqtt_message));
  mock_message->topic      = "Topic1/item1";
  mock_message->payload    = (void *)testmessage;
  mock_message->payloadlen = strlen(testmessage);
  mock_message->qos        = 0;
  mock_message->retain     = false;

  mqtt_client_test_on_message(NULL, client_instance, mock_message);
  free(mock_message);
  // check that the two registered functions are called
  TEST_ASSERT_EQUAL_INT(2, on_cb_count);
  TEST_ASSERT_TRUE_MESSAGE(
    callback1_called,
    "test_callback2 is called since it is 'Topic1/item1' message is published");
  TEST_ASSERT_TRUE_MESSAGE(
    callback2_called,
    "test_callback2 is called since it is 'Topic1/item1' message is published");

  callback1_called = false;
  callback2_called = false;
  // unsubscribe "Topic1/#"
  {
    mqtt_wrapper_unsubscribe_ExpectAndReturn(connection_instance_mock,
                                             NULL,
                                             "Topic1/#",
                                             SL_STATUS_OK);
  }
  mqtt_client_test_on_connect(NULL,
                              client_instance,
                              0);  // Connecting succeeded.

  mqtt_client_unsubscribe(client_instance, "Topic1/#", callback_test1);

  struct mqtt_message *mock_message_2
    = (struct mqtt_message *)malloc(sizeof(struct mqtt_message));
  mock_message_2->topic      = "Topic1/item1";
  mock_message_2->payload    = (void *)testmessage;
  mock_message_2->payloadlen = strlen(testmessage);
  mock_message_2->qos        = 0;
  mock_message_2->retain     = false;

  mqtt_client_test_on_message(NULL, client_instance, mock_message_2);
  free(mock_message_2);

  TEST_ASSERT_FALSE_MESSAGE(
    callback1_called,
    "test_callback1 is not called since it is 'Toipc1/#' is unsubscribed");
  TEST_ASSERT_TRUE_MESSAGE(
    callback2_called,
    "test_callback2 is called since it is 'Topic1/item1' message is published");

  mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
}
}
