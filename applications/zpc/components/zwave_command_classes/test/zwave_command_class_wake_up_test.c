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
#include "zwave_command_class_wake_up.h"
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zpc_config.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_command_class_wake_up_types.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_tx_mock.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "zwave_network_management_mock.h"
#include "attribute_timeouts_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_WAKE_UP_##type

// Static variables
static attribute_resolver_function_t wake_up_set                   = NULL;
static attribute_resolver_function_t wake_up_get                   = NULL;
static attribute_resolver_function_t wake_up_capabilities_get      = NULL;
static void (*send_wake_up_no_more)(attribute_store_node_t)        = NULL;
static void (*configure_wake_up_interval)(attribute_store_node_t)  = NULL;
static zpc_resolver_event_notification_function_t on_send_complete = NULL;
static zwave_command_handler_t wake_up_handler                     = {};
static bool sis_test_configuration                                 = true;

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

static uint8_t u8_value   = 0;
static uint32_t u32_value = 0;

static zpc_config_t my_test_configuration = {};

// Manual mocks
const zpc_config_t *zpc_get_config()
{
  return &my_test_configuration;
}

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(SETTING)) {
    wake_up_set = set_func;
    wake_up_get = get_func;
  } else if (node_type == ATTRIBUTE(CAPABILITIES)) {
    TEST_ASSERT_NULL(set_func);
    wake_up_capabilities_get = get_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }

  return SL_STATUS_OK;
}

void attribute_resolver_set_resolution_listener_stub(
  attribute_store_node_t node,
  attribute_resolver_callback_t callback,
  int cmock_num_calls)
{
  if (attribute_store_get_node_type(node)
      == ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES) {
    configure_wake_up_interval = callback;
  } else if (attribute_store_get_node_type(node) == ATTRIBUTE_NODE_ID) {
    send_wake_up_no_more = callback;
  } else {
    TEST_FAIL_MESSAGE("Unexpected Resolution listener registration");
  }
}

bool zwave_network_management_is_zpc_sis_stub(int cmock_num_calls)
{
  return sis_test_configuration;
}

zwave_node_id_t zwave_network_management_get_node_id_stub(int cmock_num_calls)
{
  return zpc_node_id;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  wake_up_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
                    wake_up_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_WAKE_UP, wake_up_handler.command_class);
  TEST_ASSERT_EQUAL(3, wake_up_handler.version);
  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  TEST_ASSERT_NULL(wake_up_handler.support_handler);
  TEST_ASSERT_FALSE(wake_up_handler.manual_security_validation);

  return SL_STATUS_OK;
}

static sl_status_t register_send_event_handler_stub(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function,
  int cmock_num_calls)
{
  on_send_complete = function;
  TEST_ASSERT_NOT_NULL(on_send_complete);
  TEST_ASSERT_EQUAL(ATTRIBUTE(SETTING), type);
  return SL_STATUS_OK;
}

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

static void zwave_command_class_wake_up_init_verification()
{
  // Network management stubs
  zwave_network_management_is_zpc_sis_Stub(
    &zwave_network_management_is_zpc_sis_stub);

  zwave_network_management_get_node_id_Stub(
    &zwave_network_management_get_node_id_stub);

  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  attribute_resolver_set_resolution_listener_Stub(
    &attribute_resolver_set_resolution_listener_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // On send complete handler
  register_send_event_handler_Stub(&register_send_event_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_wake_up_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size        = 0;
  u8_value                   = 0;
  u32_value                  = 0;
  wake_up_set                = NULL;
  wake_up_get                = NULL;
  wake_up_capabilities_get   = NULL;
  send_wake_up_no_more       = NULL;
  configure_wake_up_interval = NULL;
  on_send_complete           = NULL;
  sis_test_configuration     = true;
  memset(&my_test_configuration, 0, sizeof(zpc_config_t));
  memset(&wake_up_handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_wake_up_init_verification();
}

void test_zwave_command_class_wake_up_set()
{
  // Create a Setting node under the endpoint
  attribute_store_node_t setting_node
    = attribute_store_add_node(ATTRIBUTE(SETTING), endpoint_id_node);
  attribute_store_node_t wake_up_node_id_node
    = attribute_store_add_node(ATTRIBUTE(NODE_ID), setting_node);
  attribute_store_node_t wake_up_interval_node
    = attribute_store_add_node(ATTRIBUTE(INTERVAL), setting_node);

  zwave_node_id_t destination = 23;
  wake_up_interval_t interval = (12 << 16) | (13 << 8) | 14;
  attribute_store_set_desired(wake_up_node_id_node,
                              &destination,
                              sizeof(destination));
  attribute_store_set_desired(wake_up_interval_node,
                              &interval,
                              sizeof(interval));

  // Now ask for a Set Command:
  TEST_ASSERT_NOT_NULL(wake_up_set);
  wake_up_set(setting_node, received_frame, &received_frame_size);
  const uint8_t expected_frame[] = {COMMAND_CLASS_WAKE_UP,
                                    WAKE_UP_INTERVAL_SET,
                                    (uint8_t)((interval >> 16) & 0xFF),
                                    (uint8_t)((interval >> 8) & 0xFF),
                                    (uint8_t)((interval >> 0) & 0xFF),
                                    (uint8_t)destination};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);

  // Simulate a Send data callback. Get rules are ignored:
  on_send_complete(setting_node,
                   RESOLVER_GET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(wake_up_node_id_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(wake_up_interval_node,
                                                     REPORTED_ATTRIBUTE));

  // Supervision Working is also ignored
  on_send_complete(setting_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(wake_up_node_id_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(wake_up_interval_node,
                                                     REPORTED_ATTRIBUTE));

  // Supervision Success sets the reported
  on_send_complete(setting_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(setting_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(wake_up_node_id_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(wake_up_interval_node,
                                                    REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(setting_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(wake_up_node_id_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(wake_up_interval_node, DESIRED_ATTRIBUTE));

  // Any other code clears the desired for nodeid/interval and both for the setting
  on_send_complete(setting_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_FAIL);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(setting_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(setting_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(wake_up_node_id_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(wake_up_interval_node, DESIRED_ATTRIBUTE));
}

void test_zwave_command_class_wake_up_set_undefined_node_id()
{
  // Create a Setting node under the endpoint
  attribute_store_node_t setting_node
    = attribute_store_add_node(ATTRIBUTE(SETTING), endpoint_id_node);
  attribute_store_node_t wake_up_interval_node
    = attribute_store_add_node(ATTRIBUTE(INTERVAL), setting_node);

  wake_up_interval_t interval = (12 << 16) | (13 << 8) | 14;

  attribute_store_set_desired(wake_up_interval_node,
                              &interval,
                              sizeof(interval));

  // Now ask for a Set Command:
  TEST_ASSERT_NOT_NULL(wake_up_set);
  wake_up_set(setting_node, received_frame, &received_frame_size);
  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_zwave_command_class_wake_up_set_undefined_interval()
{
  // Create a Setting node under the endpoint
  attribute_store_node_t setting_node
    = attribute_store_add_node(ATTRIBUTE(SETTING), endpoint_id_node);
  attribute_store_node_t wake_up_node_id_node
    = attribute_store_add_node(ATTRIBUTE(NODE_ID), setting_node);

  zwave_node_id_t destination = 23;
  attribute_store_set_desired(wake_up_node_id_node,
                              &destination,
                              sizeof(destination));

  // Now ask for a Set Command:
  TEST_ASSERT_NOT_NULL(wake_up_set);
  wake_up_set(setting_node, received_frame, &received_frame_size);
  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_zwave_command_class_wake_up_get()
{
  // No Need for anything in the attribute store here.
  // Now ask for a Get Command:
  TEST_ASSERT_NOT_NULL(wake_up_get);
  wake_up_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_WAKE_UP, WAKE_UP_INTERVAL_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_wake_up_capabilities_get_v1()
{
  // Create a VALUE node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  zwave_cc_version_t version                     = 1;
  my_test_configuration.default_wake_up_interval = 20;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(CAPABILITIES),
                                             0);

  // No Need for anything in the attribute store here.
  // Now ask for a Get Command:
  TEST_ASSERT_NOT_NULL(wake_up_capabilities_get);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    wake_up_capabilities_get(wake_up_capabilities_node,
                                             received_frame,
                                             &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);

  // const uint8_t expected_frame[]
  //   = {COMMAND_CLASS_WAKE_UP, WAKE_UP_INTERVAL_CAPABILITIES_GET_V2};
  //TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  //TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
  //                              received_frame,
  //                              received_frame_size);

  // Verify the values
  wake_up_interval_t minimum_interval = 1;
  wake_up_interval_t maximum_interval = 1;
  wake_up_interval_t default_interval = 1;
  wake_up_interval_t interval_step    = 0;

  attribute_store_node_t minimum_interval_interval_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(MINIMUM_INTERVAL),
                                             0);
  attribute_store_get_reported(minimum_interval_interval_node,
                               &minimum_interval,
                               sizeof(minimum_interval));
  TEST_ASSERT_EQUAL(0, minimum_interval);

  attribute_store_node_t maximum_interval_interval_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(MAXIMUM_INTERVAL),
                                             0);
  attribute_store_get_reported(maximum_interval_interval_node,
                               &maximum_interval,
                               sizeof(maximum_interval));
  TEST_ASSERT_EQUAL(16777215, maximum_interval);

  attribute_store_node_t default_interval_interval_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(DEFAULT_INTERVAL),
                                             0);
  attribute_store_get_reported(default_interval_interval_node,
                               &default_interval,
                               sizeof(default_interval));
  TEST_ASSERT_EQUAL(my_test_configuration.default_wake_up_interval,
                    default_interval);

  attribute_store_node_t interval_step_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(INTERVAL_STEP),
                                             0);
  attribute_store_get_reported(interval_step_node,
                               &interval_step,
                               sizeof(interval_step));
  TEST_ASSERT_EQUAL(1, interval_step);

  attribute_store_node_t wake_up_bitmask_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(CAPABILITIES_BITMASK),
                                             0);

  wake_up_bitmask_t wake_up_bitmask = 0xFF;
  attribute_store_get_reported(wake_up_bitmask_node,
                               &wake_up_bitmask,
                               sizeof(wake_up_bitmask));
  TEST_ASSERT_EQUAL(0, wake_up_bitmask);
}

void test_zwave_command_class_wake_up_capabilities_get_v2()
{
  // Create a VALUE node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  zwave_cc_version_t version                     = 2;
  my_test_configuration.default_wake_up_interval = 20;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(CAPABILITIES),
                                             0);

  // No Need for anything in the attribute store here.
  // Now ask for a Get Command:
  TEST_ASSERT_NOT_NULL(wake_up_capabilities_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    wake_up_capabilities_get(wake_up_capabilities_node,
                                             received_frame,
                                             &received_frame_size));
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_WAKE_UP, WAKE_UP_INTERVAL_CAPABILITIES_GET_V2};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);

  // Verify the values
  attribute_store_node_t wake_up_bitmask_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(CAPABILITIES_BITMASK),
                                             0);
  wake_up_bitmask_t wake_up_bitmask = 0xFF;
  attribute_store_get_reported(wake_up_bitmask_node,
                               &wake_up_bitmask,
                               sizeof(wake_up_bitmask));
  TEST_ASSERT_EQUAL(0, wake_up_bitmask);
}

void test_zwave_command_class_wake_up_capabilities_get_v3()
{
  // Create a VALUE node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  zwave_cc_version_t version                     = 3;
  my_test_configuration.default_wake_up_interval = 20;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(CAPABILITIES),
                                             0);

  // No Need for anything in the attribute store here.
  // Now ask for a Get Command:
  TEST_ASSERT_NOT_NULL(wake_up_capabilities_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    wake_up_capabilities_get(wake_up_capabilities_node,
                                             received_frame,
                                             &received_frame_size));
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_WAKE_UP, WAKE_UP_INTERVAL_CAPABILITIES_GET_V2};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_wake_up_version_1_supporting_node()
{
  // Create a VALUE node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  // Nothing should be created as long as version is undefined
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version                     = 1;
  my_test_configuration.default_wake_up_interval = 20;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // V1 default capabilities should have been created:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(CAPABILITIES),
                                             0);
  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SETTING),
                                             0);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                        wake_up_capabilities_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, wake_up_setting_node);
}

void test_zwave_command_class_wake_up_version_2_supporting_node()
{
  // Create a version node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  // Nothing should be created as long as version is undefined
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 2;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // V2 default capabilities should have been created:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(CAPABILITIES),
                                             0);
  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SETTING),
                                             0);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                        wake_up_capabilities_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, wake_up_setting_node);

  // Verify the values
  attribute_store_node_t minimum_interval_interval_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(MINIMUM_INTERVAL),
                                             0);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(minimum_interval_interval_node,
                                     REPORTED_ATTRIBUTE));

  attribute_store_node_t maximum_interval_interval_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(MAXIMUM_INTERVAL),
                                             0);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(maximum_interval_interval_node,
                                     REPORTED_ATTRIBUTE));

  attribute_store_node_t interval_step_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(INTERVAL_STEP),
                                             0);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(interval_step_node, REPORTED_ATTRIBUTE));

  // Configure fake capabilities (1-13-25), no default defined:
  wake_up_interval_t minimum_interval = 1;
  wake_up_interval_t maximum_interval = 25;
  wake_up_interval_t interval_step    = 12;

  attribute_store_set_reported(minimum_interval_interval_node,
                               &minimum_interval,
                               sizeof(minimum_interval));
  attribute_store_set_reported(maximum_interval_interval_node,
                               &maximum_interval,
                               sizeof(maximum_interval));
  attribute_store_set_reported(interval_step_node,
                               &interval_step,
                               sizeof(interval_step));

  // Ask to configure the Wake Up Interval:
  my_test_configuration.default_wake_up_interval = 20;
  TEST_ASSERT_NOT_NULL(configure_wake_up_interval);
  attribute_resolver_clear_resolution_listener_Expect(
    wake_up_capabilities_node,
    configure_wake_up_interval);
  configure_wake_up_interval(wake_up_capabilities_node);
  // Verify if the desired values are matching what we expect:
  attribute_store_node_t wake_up_node_id_node
    = attribute_store_get_node_child_by_type(wake_up_setting_node,
                                             ATTRIBUTE(NODE_ID),
                                             0);

  attribute_store_node_t wake_up_interval_node
    = attribute_store_get_node_child_by_type(wake_up_setting_node,
                                             ATTRIBUTE(INTERVAL),
                                             0);

  wake_up_interval_t configured_interval = 1;
  attribute_store_get_desired(wake_up_interval_node,
                              &configured_interval,
                              sizeof(configured_interval));
  TEST_ASSERT_EQUAL(13, configured_interval);

  zwave_node_id_t configured_node_id = 0xFF;
  attribute_store_get_desired(wake_up_node_id_node,
                              &configured_node_id,
                              sizeof(configured_node_id));
  TEST_ASSERT_EQUAL(zpc_node_id, configured_node_id);

  // Ask to configure the Wake Up Interval, again with a default higher than max:
  attribute_store_undefine_desired(wake_up_interval_node);
  my_test_configuration.default_wake_up_interval = 300;
  TEST_ASSERT_NOT_NULL(configure_wake_up_interval);
  attribute_resolver_clear_resolution_listener_Expect(
    wake_up_capabilities_node,
    configure_wake_up_interval);
  configure_wake_up_interval(wake_up_capabilities_node);
  // Verify if the desired values are matching what we expect:
  configured_interval = 1;
  attribute_store_get_desired(wake_up_interval_node,
                              &configured_interval,
                              sizeof(configured_interval));
  TEST_ASSERT_EQUAL(25, configured_interval);

  // Ask to configure the Wake Up Interval, we do not re-configure if the
  // desired is set:
  my_test_configuration.default_wake_up_interval = 0;
  TEST_ASSERT_NOT_NULL(configure_wake_up_interval);
  attribute_resolver_clear_resolution_listener_Expect(
    wake_up_capabilities_node,
    configure_wake_up_interval);
  configure_wake_up_interval(wake_up_capabilities_node);
  // Verify if the desired values are matching what we expect:
  configured_interval = 0;
  attribute_store_get_desired(wake_up_interval_node,
                              &configured_interval,
                              sizeof(configured_interval));
  TEST_ASSERT_EQUAL(25, configured_interval);

  // Ask to configure the Wake Up Interval, again with a default lower than min:
  attribute_store_undefine_desired(wake_up_interval_node);
  my_test_configuration.default_wake_up_interval = 0;
  TEST_ASSERT_NOT_NULL(configure_wake_up_interval);
  attribute_resolver_clear_resolution_listener_Expect(
    wake_up_capabilities_node,
    configure_wake_up_interval);
  configure_wake_up_interval(wake_up_capabilities_node);
  // Verify if the desired values are matching what we expect:
  configured_interval = 0;
  attribute_store_get_desired(wake_up_interval_node,
                              &configured_interval,
                              sizeof(configured_interval));
  TEST_ASSERT_EQUAL(1, configured_interval);

  // Ask to configure the Wake Up Interval, when we are not the sis
  attribute_store_undefine_desired(wake_up_interval_node);
  my_test_configuration.default_wake_up_interval = 300;
  sis_test_configuration                         = false;
  TEST_ASSERT_NOT_NULL(configure_wake_up_interval);
  attribute_resolver_clear_resolution_listener_Expect(
    wake_up_capabilities_node,
    configure_wake_up_interval);
  configure_wake_up_interval(wake_up_capabilities_node);
  // Verify if the desired values has not changed
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(wake_up_interval_node, DESIRED_ATTRIBUTE));

  // Mess up with the interval step, see that we don't divide by 0
  attribute_store_undefine_desired(wake_up_interval_node);
  interval_step = 0;
  attribute_store_set_reported(interval_step_node,
                               &interval_step,
                               sizeof(interval_step));
  my_test_configuration.default_wake_up_interval = 15;
  sis_test_configuration                         = true;
  TEST_ASSERT_NOT_NULL(configure_wake_up_interval);
  attribute_resolver_clear_resolution_listener_Expect(
    wake_up_capabilities_node,
    configure_wake_up_interval);
  configure_wake_up_interval(wake_up_capabilities_node);
  configured_interval = 0;
  attribute_store_get_desired(wake_up_interval_node,
                              &configured_interval,
                              sizeof(configured_interval));
  TEST_ASSERT_EQUAL(my_test_configuration.default_wake_up_interval,
                    configured_interval);

  // Less than min for default:
  attribute_store_undefine_desired(wake_up_interval_node);
  my_test_configuration.default_wake_up_interval = 0;
  sis_test_configuration                         = true;
  TEST_ASSERT_NOT_NULL(configure_wake_up_interval);
  attribute_resolver_clear_resolution_listener_Expect(
    wake_up_capabilities_node,
    configure_wake_up_interval);
  configure_wake_up_interval(wake_up_capabilities_node);
  configured_interval = 0;
  attribute_store_get_desired(wake_up_interval_node,
                              &configured_interval,
                              sizeof(configured_interval));
  TEST_ASSERT_EQUAL(1, configured_interval);

  // More than max for default:
  attribute_store_undefine_desired(wake_up_interval_node);
  my_test_configuration.default_wake_up_interval = 4200;
  sis_test_configuration                         = true;
  TEST_ASSERT_NOT_NULL(configure_wake_up_interval);
  attribute_resolver_clear_resolution_listener_Expect(
    wake_up_capabilities_node,
    configure_wake_up_interval);
  configure_wake_up_interval(wake_up_capabilities_node);
  configured_interval = 0;
  attribute_store_get_desired(wake_up_interval_node,
                              &configured_interval,
                              sizeof(configured_interval));
  TEST_ASSERT_EQUAL(25, configured_interval);
}

void test_zwave_command_class_wake_up_no_more_information()
{
  // Create a version node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  // Nothing should be created as long as version is undefined
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // V1 default capabilities should have been created:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  // Try to ask sending a Wake Up No More. Won't happen if we are not the destination
  TEST_ASSERT_NOT_NULL(send_wake_up_no_more);
  attribute_resolver_clear_resolution_listener_Expect(node_id_node,
                                                      send_wake_up_no_more);
  attribute_resolver_pause_node_resolution_Expect(node_id_node);
  send_wake_up_no_more(node_id_node);

  // Now Configure ourselves as the Wake Up Destination
  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SETTING),
                                             0);
  attribute_store_node_t wake_up_node_id_node
    = attribute_store_get_node_child_by_type(wake_up_setting_node,
                                             ATTRIBUTE(NODE_ID),
                                             0);
  attribute_store_set_reported(wake_up_node_id_node,
                               &zpc_node_id,
                               sizeof(zpc_node_id));

  // And check that pause the node and send a Wake Up No More.
  attribute_resolver_clear_resolution_listener_Expect(node_id_node,
                                                      send_wake_up_no_more);
  attribute_resolver_pause_node_resolution_Expect(node_id_node);
  zwave_network_management_is_protocol_sending_frames_to_node_ExpectAndReturn(
    node_id,
    false);
  zwave_tx_has_frames_for_node_ExpectAndReturn(node_id, false);
  zwave_node_id_t remote_node_id = 0;
  attribute_store_get_reported(node_id_node,
                               &remote_node_id,
                               sizeof(remote_node_id));
  zwave_tx_scheme_get_node_connection_info_Expect(remote_node_id, 0, NULL);
  zwave_tx_scheme_get_node_connection_info_IgnoreArg_connection_info();
  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    15 * 1000,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     2,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_tx_options();

  send_wake_up_no_more(node_id_node);
}

void test_postpone_wake_up_no_more_information()
{
  // Create a version node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  // Nothing should be created as long as version is undefined
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // V1 default capabilities should have been created:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  // Now Configure ourselves as the Wake Up Destination
  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SETTING),
                                             0);
  attribute_store_node_t wake_up_node_id_node
    = attribute_store_get_node_child_by_type(wake_up_setting_node,
                                             ATTRIBUTE(NODE_ID),
                                             0);
  attribute_store_set_reported(wake_up_node_id_node,
                               &zpc_node_id,
                               sizeof(zpc_node_id));

  // If we have return routes to assign, it will postpone the pausing/sending of
  // the wake up no more message
  attribute_resolver_clear_resolution_listener_Expect(node_id_node,
                                                      send_wake_up_no_more);
  zwave_network_management_is_protocol_sending_frames_to_node_ExpectAndReturn(
    node_id,
    true);
  attribute_timeout_set_callback_ExpectAndReturn(node_id_node,
                                                 1000,
                                                 send_wake_up_no_more,
                                                 SL_STATUS_OK);

  send_wake_up_no_more(node_id_node);

  // Same if we have tx frames in the queue
  attribute_resolver_clear_resolution_listener_Expect(node_id_node,
                                                      send_wake_up_no_more);
  zwave_network_management_is_protocol_sending_frames_to_node_ExpectAndReturn(
    node_id,
    false);
  zwave_tx_has_frames_for_node_ExpectAndReturn(node_id, true);
  attribute_timeout_set_callback_ExpectAndReturn(node_id_node,
                                                 1000,
                                                 send_wake_up_no_more,
                                                 SL_STATUS_OK);

  send_wake_up_no_more(node_id_node);
}

void test_zwave_command_class_support_wake_up_on_demand()
{
  // Wake Up On demand is assumed to be supported only on EP 0
  zwave_endpoint_id_t supporting_node_endpoint_id = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &supporting_node_endpoint_id,
                               sizeof(supporting_node_endpoint_id));

  TEST_ASSERT_FALSE(
    zwave_command_class_wake_up_supports_wake_up_on_demand(node_id_node));

  //Create a Wake Up Version:
  attribute_store_node_t wake_up_version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(wake_up_version_node, &version, sizeof(version));

  TEST_ASSERT_FALSE(
    zwave_command_class_wake_up_supports_wake_up_on_demand(node_id_node));

  //Create a Supervision Version:
  attribute_store_node_t supervision_version_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_SUPERVISION_VERSION,
                               endpoint_id_node);
  version = 2;
  attribute_store_set_reported(supervision_version_node,
                               &version,
                               sizeof(version));

  TEST_ASSERT_FALSE(
    zwave_command_class_wake_up_supports_wake_up_on_demand(node_id_node));

  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(CAPABILITIES),
                                             0);

  attribute_store_node_t wake_up_bitmask_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(CAPABILITIES_BITMASK),
                                             0);

  wake_up_bitmask_t wake_up_bitmask = 0;
  attribute_store_set_reported(wake_up_bitmask_node,
                               &wake_up_bitmask,
                               sizeof(wake_up_bitmask));

  TEST_ASSERT_FALSE(
    zwave_command_class_wake_up_supports_wake_up_on_demand(node_id_node));

  wake_up_bitmask = 0xFE;
  attribute_store_set_reported(wake_up_bitmask_node,
                               &wake_up_bitmask,
                               sizeof(wake_up_bitmask));

  TEST_ASSERT_FALSE(
    zwave_command_class_wake_up_supports_wake_up_on_demand(node_id_node));

  wake_up_bitmask = 0x1;
  attribute_store_set_reported(wake_up_bitmask_node,
                               &wake_up_bitmask,
                               sizeof(wake_up_bitmask));

  TEST_ASSERT_TRUE(
    zwave_command_class_wake_up_supports_wake_up_on_demand(node_id_node));
}

void test_zwave_command_class_wake_up_incoming_frame_too_short()
{
  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));
  const uint8_t incoming_frame[] = {COMMAND_CLASS_WAKE_UP};

  // Too short will do nothing.
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    wake_up_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));
}

void test_zwave_command_class_wake_up_incoming_unknown_command()
{
  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));
  const uint8_t incoming_frame[] = {COMMAND_CLASS_WAKE_UP, 0xFF};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    wake_up_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));
}

void test_zwave_command_class_wake_up_incoming_wrong_command_class()
{
  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));
  const uint8_t incoming_frame[] = {0xAA, 0xFF};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    wake_up_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));
}

void test_zwave_command_class_wake_up_incoming_wake_up_notification()
{
  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_WAKE_UP, WAKE_UP_NOTIFICATION};

  TEST_ASSERT_NULL(send_wake_up_no_more);

  attribute_resolver_resume_node_resolution_Expect(node_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    wake_up_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Wake Up No More resolution listener must have been registerd
  TEST_ASSERT_NOT_NULL(send_wake_up_no_more);
}

void test_zwave_command_class_wake_up_incoming_wake_up_report()
{
  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  //Create a Wake Up Version, it will create capabilities/settings:
  attribute_store_node_t wake_up_version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(wake_up_version_node, &version, sizeof(version));

  wake_up_interval_t expected_interval = 283741;
  zwave_node_id_t expected_node_id     = 232;
  const uint8_t incoming_frame[]       = {COMMAND_CLASS_WAKE_UP,
                                          WAKE_UP_INTERVAL_REPORT,
                                          (expected_interval >> 16) & 0xFF,
                                          (expected_interval >> 8) & 0xFF,
                                          (expected_interval)&0xFF,
                                          (uint8_t)(expected_node_id)};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    wake_up_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Verifications:
  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SETTING),
                                             0);

  attribute_store_node_t wake_up_interval_node
    = attribute_store_get_node_child_by_type(wake_up_setting_node,
                                             ATTRIBUTE(INTERVAL),
                                             0);
  wake_up_interval_t received_interval = 0;
  attribute_store_get_reported(wake_up_interval_node,
                               &received_interval,
                               sizeof(received_interval));
  TEST_ASSERT_EQUAL(expected_interval, received_interval);

  attribute_store_node_t wake_up_node_id_node
    = attribute_store_get_node_child_by_type(wake_up_setting_node,
                                             ATTRIBUTE(NODE_ID),
                                             0);
  zwave_node_id_t received_node_id = 0;
  attribute_store_get_reported(wake_up_node_id_node,
                               &received_node_id,
                               sizeof(received_node_id));
  TEST_ASSERT_EQUAL(expected_node_id, received_node_id);

  command_status_values_t received_status = 99;
  attribute_store_get_reported(wake_up_setting_node,
                               &received_status,
                               sizeof(received_status));
  TEST_ASSERT_EQUAL(FINAL_STATE, received_status);
}

void test_zwave_command_class_wake_up_incoming_wake_up_report_too_short()
{
  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  //Create a Wake Up Version, it will create capabilities/settings:
  attribute_store_node_t wake_up_version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(wake_up_version_node, &version, sizeof(version));

  wake_up_interval_t expected_interval = 283741;
  const uint8_t incoming_frame[]       = {COMMAND_CLASS_WAKE_UP,
                                          WAKE_UP_INTERVAL_REPORT,
                                          (expected_interval >> 16) & 0xFF,
                                          (expected_interval >> 8) & 0xFF,
                                          (expected_interval)&0xFF};

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    wake_up_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Verifications:
  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SETTING),
                                             0);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(wake_up_setting_node, REPORTED_ATTRIBUTE));
}

void test_zwave_command_class_wake_up_incoming_wake_up_capabilities_report_v3()
{
  // Wake Up On demand is assumed to be supported only on EP 0. We use
  // that later in the test
  zwave_endpoint_id_t supporting_node_endpoint_id = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &supporting_node_endpoint_id,
                               sizeof(supporting_node_endpoint_id));

  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  //Create a Wake Up Version, it will create capabilities/settings:
  attribute_store_node_t wake_up_version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(wake_up_version_node, &version, sizeof(version));
  attribute_store_node_t supervision_version_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_SUPERVISION_VERSION,
                               endpoint_id_node);
  attribute_store_set_reported(supervision_version_node,
                               &version,
                               sizeof(version));

  wake_up_interval_t expected_min_interval     = 123456;
  wake_up_interval_t expected_max_interval     = 123458;
  wake_up_interval_t expected_default_interval = 123457;
  wake_up_interval_t expected_interval_step    = 123451;
  wake_up_bitmask_t expected_bitmask           = 0xEF;
  const uint8_t incoming_frame[]               = {COMMAND_CLASS_WAKE_UP,
                                                  WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2,
                                                  (expected_min_interval >> 16) & 0xFF,
                                                  (expected_min_interval >> 8) & 0xFF,
                                                  (expected_min_interval)&0xFF,
                                                  (expected_max_interval >> 16) & 0xFF,
                                                  (expected_max_interval >> 8) & 0xFF,
                                                  (expected_max_interval)&0xFF,
                                                  (expected_default_interval >> 16) & 0xFF,
                                                  (expected_default_interval >> 8) & 0xFF,
                                                  (expected_default_interval)&0xFF,
                                                  (expected_interval_step >> 16) & 0xFF,
                                                  (expected_interval_step >> 8) & 0xFF,
                                                  (expected_interval_step)&0xFF,
                                                  expected_bitmask};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    wake_up_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Verifications:
  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(CAPABILITIES),
                                             0);

  attribute_store_node_t wake_up_bitmask_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(CAPABILITIES_BITMASK),
                                             0);
  wake_up_bitmask_t received_bitmask = 0;
  attribute_store_get_reported(wake_up_bitmask_node,
                               &received_bitmask,
                               sizeof(received_bitmask));
  TEST_ASSERT_EQUAL(expected_bitmask, received_bitmask);

  wake_up_interval_t received_interval = 0;
  attribute_store_node_t wake_up_min_interval_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(MINIMUM_INTERVAL),
                                             0);
  attribute_store_get_reported(wake_up_min_interval_node,
                               &received_interval,
                               sizeof(received_interval));
  TEST_ASSERT_EQUAL(expected_min_interval, received_interval);

  attribute_store_node_t wake_up_max_interval_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(MAXIMUM_INTERVAL),
                                             0);
  attribute_store_get_reported(wake_up_max_interval_node,
                               &received_interval,
                               sizeof(received_interval));
  TEST_ASSERT_EQUAL(expected_max_interval, received_interval);

  attribute_store_node_t wake_up_default_interval_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(DEFAULT_INTERVAL),
                                             0);
  attribute_store_get_reported(wake_up_default_interval_node,
                               &received_interval,
                               sizeof(received_interval));
  TEST_ASSERT_EQUAL(expected_default_interval, received_interval);

  attribute_store_node_t wake_up_interval_step_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(INTERVAL_STEP),
                                             0);
  attribute_store_get_reported(wake_up_interval_step_node,
                               &received_interval,
                               sizeof(received_interval));
  TEST_ASSERT_EQUAL(expected_interval_step, received_interval);

  command_status_values_t received_status = 99;
  attribute_store_get_reported(wake_up_capabilities_node,
                               &received_status,
                               sizeof(received_status));
  TEST_ASSERT_EQUAL(FINAL_STATE, received_status);

  // Additional test:(get_maximum_possible_interval)
  // Try to see if we use the maximum wake up if the node supports
  // Wake up on demand.
  TEST_ASSERT_NOT_NULL(configure_wake_up_interval);
  attribute_resolver_clear_resolution_listener_Expect(
    wake_up_capabilities_node,
    configure_wake_up_interval);
  configure_wake_up_interval(wake_up_capabilities_node);
  // Verify if the desired values are matching what we expect:
  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SETTING),
                                             0);

  attribute_store_node_t wake_up_interval_node
    = attribute_store_get_node_child_by_type(wake_up_setting_node,
                                             ATTRIBUTE(INTERVAL),
                                             0);
  attribute_store_get_desired(wake_up_interval_node,
                              &received_interval,
                              sizeof(received_interval));
  TEST_ASSERT_EQUAL(expected_max_interval, received_interval);

  // Additional test: (get_maximum_possible_interval)
  // Set the min to 0, check that this is used for the max
  attribute_store_undefine_desired(wake_up_interval_node);
  received_interval = 0;
  attribute_store_set_reported(wake_up_min_interval_node,
                               &received_interval,
                               sizeof(received_interval));
  attribute_resolver_clear_resolution_listener_Expect(
    wake_up_capabilities_node,
    configure_wake_up_interval);
  configure_wake_up_interval(wake_up_capabilities_node);
  // Verify if the desired values are matching what we expect:
  attribute_store_get_desired(wake_up_interval_node,
                              &received_interval,
                              sizeof(received_interval));
  TEST_ASSERT_EQUAL(0, received_interval);
}

void test_zwave_command_class_wake_up_incoming_wake_up_capabilities_report_v2()
{
  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  //Create a Wake Up Version, it will create capabilities/settings:
  attribute_store_node_t wake_up_version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(wake_up_version_node, &version, sizeof(version));

  wake_up_interval_t expected_min_interval     = 123456;
  wake_up_interval_t expected_max_interval     = 123458;
  wake_up_interval_t expected_default_interval = 123457;
  wake_up_interval_t expected_interval_step    = 123451;
  const uint8_t incoming_frame[]               = {COMMAND_CLASS_WAKE_UP,
                                                  WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2,
                                                  (expected_min_interval >> 16) & 0xFF,
                                                  (expected_min_interval >> 8) & 0xFF,
                                                  (expected_min_interval)&0xFF,
                                                  (expected_max_interval >> 16) & 0xFF,
                                                  (expected_max_interval >> 8) & 0xFF,
                                                  (expected_max_interval)&0xFF,
                                                  (expected_default_interval >> 16) & 0xFF,
                                                  (expected_default_interval >> 8) & 0xFF,
                                                  (expected_default_interval)&0xFF,
                                                  (expected_interval_step >> 16) & 0xFF,
                                                  (expected_interval_step >> 8) & 0xFF,
                                                  (expected_interval_step)&0xFF};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    wake_up_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Verifications:
  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(CAPABILITIES),
                                             0);

  wake_up_interval_t received_interval = 0;
  attribute_store_node_t wake_up_min_interval_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(MINIMUM_INTERVAL),
                                             0);
  attribute_store_get_reported(wake_up_min_interval_node,
                               &received_interval,
                               sizeof(received_interval));
  TEST_ASSERT_EQUAL(expected_min_interval, received_interval);

  attribute_store_node_t wake_up_max_interval_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(MAXIMUM_INTERVAL),
                                             0);
  attribute_store_get_reported(wake_up_max_interval_node,
                               &received_interval,
                               sizeof(received_interval));
  TEST_ASSERT_EQUAL(expected_max_interval, received_interval);

  attribute_store_node_t wake_up_default_interval_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(DEFAULT_INTERVAL),
                                             0);
  attribute_store_get_reported(wake_up_default_interval_node,
                               &received_interval,
                               sizeof(received_interval));
  TEST_ASSERT_EQUAL(expected_default_interval, received_interval);

  attribute_store_node_t wake_up_interval_step_node
    = attribute_store_get_node_child_by_type(wake_up_capabilities_node,
                                             ATTRIBUTE(INTERVAL_STEP),
                                             0);
  attribute_store_get_reported(wake_up_interval_step_node,
                               &received_interval,
                               sizeof(received_interval));
  TEST_ASSERT_EQUAL(expected_interval_step, received_interval);

  command_status_values_t received_status = 99;
  attribute_store_get_reported(wake_up_capabilities_node,
                               &received_status,
                               sizeof(received_status));
  TEST_ASSERT_EQUAL(FINAL_STATE, received_status);
}

void test_zwave_command_class_wake_up_incoming_wake_up_capabilities_report_too_short()
{
  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  //Create a Wake Up Version, it will create capabilities/settings:
  attribute_store_node_t wake_up_version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(wake_up_version_node, &version, sizeof(version));

  wake_up_interval_t expected_min_interval     = 123456;
  wake_up_interval_t expected_max_interval     = 123458;
  wake_up_interval_t expected_default_interval = 123457;
  wake_up_interval_t expected_interval_step    = 123451;
  const uint8_t incoming_frame[]               = {COMMAND_CLASS_WAKE_UP,
                                                  WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2,
                                                  (expected_min_interval >> 16) & 0xFF,
                                                  (expected_min_interval >> 8) & 0xFF,
                                                  (expected_min_interval)&0xFF,
                                                  (expected_max_interval >> 16) & 0xFF,
                                                  (expected_max_interval >> 8) & 0xFF,
                                                  (expected_max_interval)&0xFF,
                                                  (expected_default_interval >> 16) & 0xFF,
                                                  (expected_default_interval >> 8) & 0xFF,
                                                  (expected_default_interval)&0xFF,
                                                  (expected_interval_step >> 16) & 0xFF,
                                                  (expected_interval_step >> 8) & 0xFF};

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    wake_up_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Verifications:
  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(CAPABILITIES),
                                             0);

  TEST_ASSERT_FALSE(attribute_store_is_value_defined(wake_up_capabilities_node,
                                                     REPORTED_ATTRIBUTE));
}

void test_zwave_command_class_wake_up_verify_wake_up_setting_trigger()
{
  //Create a Wake Up Version, it will create capabilities/settings:
  attribute_store_node_t wake_up_version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(wake_up_version_node, &version, sizeof(version));
  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SETTING),
                                             0);

  attribute_store_node_t wake_up_interval_node
    = attribute_store_get_node_child_by_type(wake_up_setting_node,
                                             ATTRIBUTE(INTERVAL),
                                             0);
  attribute_store_node_t wake_up_node_id_node
    = attribute_store_get_node_child_by_type(wake_up_setting_node,
                                             ATTRIBUTE(NODE_ID),
                                             0);
  wake_up_interval_t interval = 20;
  attribute_store_set_reported(wake_up_interval_node,
                               &interval,
                               sizeof(interval));
  zwave_node_id_t node_id = 15;
  attribute_store_set_reported(wake_up_node_id_node, &node_id, sizeof(node_id));

  interval += 1;
  attribute_store_set_desired(wake_up_interval_node,
                              &interval,
                              sizeof(interval));

  // Verify that Setting is now Mismatched (NEEDS_ONE_COMMAND/FINAL_STATS):
  command_status_values_t received_status = 99;
  attribute_store_get_reported(wake_up_setting_node,
                               &received_status,
                               sizeof(received_status));
  TEST_ASSERT_EQUAL(NEEDS_ONE_COMMAND, received_status);
  attribute_store_get_desired(wake_up_setting_node,
                              &received_status,
                              sizeof(received_status));
  TEST_ASSERT_EQUAL(FINAL_STATE, received_status);

  // This is for test coverage, nothing to verify really.
  attribute_store_delete_node(wake_up_interval_node);
}

void test_zwave_command_class_wake_up_verify_wake_up_capability_trigger()
{
  //Create a Wake Up Version, it will create capabilities/settings:
  attribute_store_node_t wake_up_version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(wake_up_version_node, &version, sizeof(version));
  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(CAPABILITIES),
                                             0);

  // Update the capabilities
  configure_wake_up_interval     = NULL;
  command_status_values_t status = 99;
  attribute_store_set_desired(wake_up_capabilities_node,
                              &status,
                              sizeof(status));

  TEST_ASSERT_NOT_NULL(configure_wake_up_interval);
}

void test_zwave_command_class_wake_up_verify_if_resolution_is_needed()
{
  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  //Create a Wake Up Version, it will create capabilities/settings:
  attribute_store_node_t wake_up_version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(wake_up_version_node, &version, sizeof(version));

  // Incoming report, will create reported values
  wake_up_interval_t expected_interval = 283741;
  zwave_node_id_t expected_node_id     = 232;
  const uint8_t incoming_frame[]       = {COMMAND_CLASS_WAKE_UP,
                                          WAKE_UP_INTERVAL_REPORT,
                                          (expected_interval >> 16) & 0xFF,
                                          (expected_interval >> 8) & 0xFF,
                                          (expected_interval)&0xFF,
                                          (uint8_t)(expected_node_id)};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    wake_up_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Verifications:
  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SETTING),
                                             0);

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(wake_up_setting_node, REPORTED_ATTRIBUTE));

  // Undefine the Wake up Interval, it should undefine the Setting.
  attribute_store_node_t wake_up_interval_node
    = attribute_store_get_node_child_by_type(wake_up_setting_node,
                                             ATTRIBUTE(INTERVAL),
                                             0);
  attribute_store_undefine_reported(wake_up_interval_node);
  attribute_store_set_desired(wake_up_interval_node,
                              &expected_interval,
                              sizeof(expected_interval));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(wake_up_setting_node, REPORTED_ATTRIBUTE));

  // update the desired to re-evalute if we need resolution:
  attribute_store_set_reported(wake_up_interval_node,
                               &expected_interval,
                               sizeof(expected_interval));
  expected_interval -= 1;
  attribute_store_set_desired(wake_up_interval_node,
                              &expected_interval,
                              sizeof(expected_interval));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(wake_up_setting_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(wake_up_setting_node));

  attribute_store_set_reported(wake_up_interval_node,
                               &expected_interval,
                               sizeof(expected_interval));

  attribute_store_node_t wake_up_node_id_node
    = attribute_store_get_node_child_by_type(wake_up_setting_node,
                                             ATTRIBUTE(NODE_ID),
                                             0);
  attribute_store_undefine_reported(wake_up_node_id_node);
  attribute_store_set_desired(wake_up_node_id_node,
                              &expected_node_id,
                              sizeof(expected_node_id));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(wake_up_setting_node, REPORTED_ATTRIBUTE));
  attribute_store_set_reported(wake_up_node_id_node,
                               &expected_node_id,
                               sizeof(expected_node_id));
  expected_node_id -= 1;
  attribute_store_set_desired(wake_up_node_id_node,
                              &expected_node_id,
                              sizeof(expected_node_id));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(wake_up_setting_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(wake_up_setting_node));
}