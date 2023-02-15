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
#include "unify_application_monitoring.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Unify Components
#include "uic_version.h"

// Mocks
#include "uic_mqtt_mock.h"
#include "dotdot_mqtt_mock.h"

// Private variables
static const char test_client_id[] = "test-client-id";
static mqtt_connection_callbacks_t before_disconnect_callback = NULL;
static mqtt_connection_callbacks_t after_connect_callback     = NULL;

// Stubs
void uic_mqtt_set_before_disconnect_callback_stub(
  mqtt_connection_callbacks_t callback, int cmock_num_calls)
{
  before_disconnect_callback = callback;
}
void uic_mqtt_set_after_connect_callback_stub(
  mqtt_connection_callbacks_t callback, int cmock_num_calls)
{
  after_connect_callback = callback;
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
  uic_mqtt_set_before_disconnect_callback_Stub(
    &uic_mqtt_set_before_disconnect_callback_stub);
  uic_mqtt_set_after_connect_callback_Stub(
    &uic_mqtt_set_after_connect_callback_stub);
}

void tearDown()
{
  uic_mqtt_get_client_id_StopIgnore();
  uic_mqtt_unretain_Expect("ucl/by-mqtt-client/test-client-id");
  uic_mqtt_get_client_id_ExpectAndReturn(test_client_id);
  unify_application_monitoring_teardown();
}

void test_unify_application_monitoring_not_initialized()
{
  // Do not initialize, then no publications should be made.
  unify_application_monitoring_set_application_name("Unknown Application");
}

void test_unify_application_monitoring_invalid_client_id()
{
  // No Client ID, no publications.
  uic_mqtt_get_client_id_IgnoreAndReturn(NULL);
  uic_mqtt_is_connected_to_broker_ExpectAndReturn(true);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, unify_application_monitoring_init());
}

void test_unify_application_monitoring_default_values()
{
  uic_mqtt_get_client_id_IgnoreAndReturn(test_client_id);
  uic_mqtt_is_connected_to_broker_ExpectAndReturn(false);
  uic_mqtt_dotdot_application_monitoring_application_connected_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    false,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_application_monitoring_application_name_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    "Unknown Application",
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_application_monitoring_application_version_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    UIC_VERSION,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_application_monitoring_publish_cluster_revision_Expect(
    "ucl/by-mqtt-client/test-client-id",
    APPLICATION_MONITORING_CLUSTER_REVISION);
  uic_mqtt_publish_Expect(
    "ucl/by-mqtt-client/test-client-id/ApplicationMonitoring/SupportedCommands",
    "{\"value\":[]}",
    strlen("{\"value\":[]}"),
    true);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, unify_application_monitoring_init());

  uic_mqtt_dotdot_application_monitoring_application_name_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    "Test Application",
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_application_monitoring_application_version_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    "Test Version 1.2.304.23komfs",
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  unify_application_monitoring_set_application_name("Test Application");
  unify_application_monitoring_set_application_version(
    "Test Version 1.2.304.23komfs");
}

void test_unify_application_after_connect_callback()
{
  uic_mqtt_get_client_id_IgnoreAndReturn(test_client_id);
  uic_mqtt_is_connected_to_broker_ExpectAndReturn(true);
  uic_mqtt_dotdot_application_monitoring_application_connected_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    true,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_application_monitoring_application_name_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    "Test Application",
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_application_monitoring_application_version_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    "Test Version 1.2.304.23komfs",
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_application_monitoring_publish_cluster_revision_Expect(
    "ucl/by-mqtt-client/test-client-id",
    APPLICATION_MONITORING_CLUSTER_REVISION);
  uic_mqtt_publish_Expect(
    "ucl/by-mqtt-client/test-client-id/ApplicationMonitoring/SupportedCommands",
    "{\"value\":[]}",
    strlen("{\"value\":[]}"),
    true);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, unify_application_monitoring_init());

  TEST_ASSERT_NOT_NULL(after_connect_callback);
  uic_mqtt_is_connected_to_broker_ExpectAndReturn(false);
  uic_mqtt_dotdot_application_monitoring_application_connected_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    false,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  after_connect_callback();
}

void test_unify_application_before_disconnect_callback()
{
  uic_mqtt_get_client_id_IgnoreAndReturn(test_client_id);
  uic_mqtt_is_connected_to_broker_ExpectAndReturn(true);
  uic_mqtt_dotdot_application_monitoring_application_connected_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    true,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_application_monitoring_application_name_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    "Test Application",
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_application_monitoring_application_version_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    "Test Version 1.2.304.23komfs",
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_application_monitoring_publish_cluster_revision_Expect(
    "ucl/by-mqtt-client/test-client-id",
    APPLICATION_MONITORING_CLUSTER_REVISION);
  uic_mqtt_publish_Expect(
    "ucl/by-mqtt-client/test-client-id/ApplicationMonitoring/SupportedCommands",
    "{\"value\":[]}",
    strlen("{\"value\":[]}"),
    true);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, unify_application_monitoring_init());

  TEST_ASSERT_NOT_NULL(before_disconnect_callback);
  uic_mqtt_dotdot_application_monitoring_application_connected_publish_ExpectAndReturn(
    "ucl/by-mqtt-client/test-client-id",
    false,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  before_disconnect_callback();
}