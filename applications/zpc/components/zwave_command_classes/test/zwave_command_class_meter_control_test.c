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
#include "zwave_command_class_meter_control.h"
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zpc_attribute_store_network_helper.h"
#include "zpc_attribute_store_type_registration.h"
#include "sl_log.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"
#include "zwave_command_class_meter_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_network_management_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_METER_##type

// Static variables
static attribute_resolver_function_t meter_reset         = NULL;
static attribute_resolver_function_t meter_get           = NULL;
static attribute_resolver_function_t meter_supported_get = NULL;
static zpc_resolver_event_notification_function_t on_meter_reset_complete
  = NULL;

static zwave_command_handler_t meter_handler   = {};
static zwave_controller_connection_info_t info = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(VALUE)) {
    meter_reset = set_func;
    meter_get   = get_func;
  } else if (node_type == ATTRIBUTE(SUPPORTED_SCALES)) {
    TEST_ASSERT_NULL(set_func);
    meter_supported_get = get_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  meter_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    meter_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_METER_V5, meter_handler.command_class);
  TEST_ASSERT_EQUAL(6, meter_handler.version);
  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_NULL(meter_handler.support_handler);
  TEST_ASSERT_FALSE(meter_handler.manual_security_validation);

  return SL_STATUS_OK;
}

static sl_status_t register_send_event_handler_stub(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function,
  int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(ATTRIBUTE(VALUE), type);
  on_meter_reset_complete = function;
  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

static void zwave_command_class_meter_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  register_send_event_handler_Stub(&register_send_event_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_meter_control_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size     = 0;
  meter_reset             = NULL;
  meter_get               = NULL;
  meter_supported_get     = NULL;
  on_meter_reset_complete = NULL;
  memset(&meter_handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_meter_init_verification();

  // Prepare a default connection info with the supporting node
  memset(&info, 0, sizeof(zwave_controller_connection_info_t));
  attribute_store_get_reported(node_id_node,
                               &(info.remote.node_id),
                               sizeof(info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(info.remote.endpoint_id),
                               sizeof(info.remote.endpoint_id));
}

/// Called after each and every test
void tearDown() {}

void test_meter_version_1_supporting_node()
{
  // Simulate a version 1 node.
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_METER_V5),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  const zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // 3 new attributes should be created:
  TEST_ASSERT_EQUAL(1 + 3,
                    attribute_store_get_node_child_count(endpoint_id_node));

  attribute_store_node_t reset_supported_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(RESET_SUPPORTED));
  attribute_store_node_t supported_scales_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SUPPORTED_SCALES));
  attribute_store_node_t supported_rate_types_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SUPPORTED_RATE_TYPES));

  meter_reset_supported_t reset_supported = true;
  attribute_store_get_reported(reset_supported_node,
                               &reset_supported,
                               sizeof(reset_supported));
  TEST_ASSERT_FALSE(reset_supported);

  meter_supported_rate_types_t supported_rate_types = 0xFF;
  attribute_store_get_reported(supported_rate_types_node,
                               &supported_rate_types,
                               sizeof(supported_rate_types));
  TEST_ASSERT_EQUAL(0, supported_rate_types);

  TEST_ASSERT_FALSE(attribute_store_is_value_defined(supported_scales_node,
                                                     REPORTED_ATTRIBUTE));
}

void test_meter_version_2_supporting_node()
{
  // Simulate a version 2 node.
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_METER_V5),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 0;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // nothing happens yet
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  version = 2;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // 3 new attributes should be created:
  TEST_ASSERT_EQUAL(1 + 3,
                    attribute_store_get_node_child_count(endpoint_id_node));

  attribute_store_node_t reset_supported_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(RESET_SUPPORTED));
  attribute_store_node_t supported_scales_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SUPPORTED_SCALES));
  attribute_store_node_t supported_rate_types_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SUPPORTED_RATE_TYPES));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(reset_supported_node, REPORTED_ATTRIBUTE));

  TEST_ASSERT_FALSE(attribute_store_is_value_defined(supported_scales_node,
                                                     REPORTED_ATTRIBUTE));

  TEST_ASSERT_FALSE(attribute_store_is_value_defined(supported_rate_types_node,
                                                     REPORTED_ATTRIBUTE));
}

void test_meter_supported_get_v1()
{
  // Simulate a version 1 node.
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_METER_V5),
    endpoint_id_node);

  const zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t supported_scales_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SUPPORTED_SCALES));

  TEST_ASSERT_NOT_NULL(meter_supported_get);
  meter_supported_get(supported_scales_node,
                      received_frame,
                      &received_frame_size);

  const uint8_t expected_frame[] = {COMMAND_CLASS_METER_V5, METER_GET, 0, 0};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_meter_supported_get_v2()
{
  // Simulate a version 2 node.
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_METER_V5),
    endpoint_id_node);

  const zwave_cc_version_t version = 2;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t supported_scales_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SUPPORTED_SCALES));

  TEST_ASSERT_NOT_NULL(meter_supported_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    meter_supported_get(supported_scales_node,
                                        received_frame,
                                        &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_METER_V5, METER_SUPPORTED_GET_V5};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_meter_get_low_scale()
{
  attribute_store_node_t meter_type_node
    = attribute_store_add_node(ATTRIBUTE(TYPE), endpoint_id_node);
  const meter_type_t type = 3;
  attribute_store_set_reported(meter_type_node, &type, sizeof(type));
  attribute_store_node_t scale_node
    = attribute_store_add_node(ATTRIBUTE(SCALE), meter_type_node);
  const meter_scale_t scale = 6;
  attribute_store_set_reported(scale_node, &scale, sizeof(scale));
  attribute_store_node_t rate_type_node
    = attribute_store_add_node(ATTRIBUTE(RATE_TYPE), scale_node);
  const meter_rate_type_t rate_type = 3;
  attribute_store_set_reported(rate_type_node, &rate_type, sizeof(rate_type));
  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(VALUE), rate_type_node);

  TEST_ASSERT_NOT_NULL(meter_get);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    meter_get(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_METER_V5, METER_GET, (rate_type << 6) | (scale << 3), 0};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_meter_get_high_scale()
{
  attribute_store_node_t meter_type_node
    = attribute_store_add_node(ATTRIBUTE(TYPE), endpoint_id_node);
  const meter_type_t type = 1;
  attribute_store_set_reported(meter_type_node, &type, sizeof(type));
  attribute_store_node_t scale_node
    = attribute_store_add_node(ATTRIBUTE(SCALE), meter_type_node);
  const meter_scale_t scale = 9;
  attribute_store_set_reported(scale_node, &scale, sizeof(scale));
  attribute_store_node_t rate_type_node
    = attribute_store_add_node(ATTRIBUTE(RATE_TYPE), scale_node);
  const meter_rate_type_t rate_type = 2;
  attribute_store_set_reported(rate_type_node, &rate_type, sizeof(rate_type));
  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(VALUE), rate_type_node);

  TEST_ASSERT_NOT_NULL(meter_get);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    meter_get(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_METER_V5,
                                    METER_GET,
                                    (rate_type << 6) | (7 << 3),
                                    (scale - 8)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_meter_reset_no_support()
{
  attribute_store_node_t meter_type_node
    = attribute_store_add_node(ATTRIBUTE(TYPE), endpoint_id_node);
  attribute_store_node_t scale_node
    = attribute_store_add_node(ATTRIBUTE(SCALE), meter_type_node);
  attribute_store_node_t rate_type_node
    = attribute_store_add_node(ATTRIBUTE(RATE_TYPE), scale_node);
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(rate_type_node, ATTRIBUTE(VALUE));

  TEST_ASSERT_NOT_NULL(meter_reset);
  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    meter_reset(value_node, received_frame, &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_meter_reset()
{
  const meter_reset_supported_t reset_support = true;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(RESET_SUPPORTED),
                                     &reset_support,
                                     sizeof(reset_support));

  attribute_store_node_t meter_type_node
    = attribute_store_add_node(ATTRIBUTE(TYPE), endpoint_id_node);
  const meter_type_t type = 1;
  attribute_store_set_reported(meter_type_node, &type, sizeof(type));
  attribute_store_node_t scale_node
    = attribute_store_add_node(ATTRIBUTE(SCALE), meter_type_node);
  const meter_scale_t scale = 9;
  attribute_store_set_reported(scale_node, &scale, sizeof(scale));
  attribute_store_node_t rate_type_node
    = attribute_store_add_node(ATTRIBUTE(RATE_TYPE), scale_node);
  const meter_rate_type_t rate_type = 2;
  attribute_store_set_reported(rate_type_node, &rate_type, sizeof(rate_type));
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(rate_type_node, ATTRIBUTE(VALUE));
  const meter_value_t value = 101101;
  attribute_store_set_desired(value_node, &value, sizeof(value));
  attribute_store_node_t precision_node
    = attribute_store_get_first_child_by_type(rate_type_node,
                                              ATTRIBUTE(PRECISION));
  const meter_precision_t precision = 5;
  attribute_store_set_desired(precision_node, &precision, sizeof(precision));

  TEST_ASSERT_NOT_NULL(meter_reset);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    meter_reset(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_METER_V5,
                                    METER_RESET_V5,
                                    (1 << 7) | (rate_type << 5) | type,
                                    (precision << 5) | (3 << 3) | 4,
                                    (value >> 24) & 0xFF,
                                    (value >> 16) & 0xFF,
                                    (value >> 8) & 0xFF,
                                    value & 0xFF,
                                    scale - 8};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_meter_reset_scale_2()
{
  const meter_reset_supported_t reset_support = true;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(RESET_SUPPORTED),
                                     &reset_support,
                                     sizeof(reset_support));

  attribute_store_node_t meter_type_node
    = attribute_store_add_node(ATTRIBUTE(TYPE), endpoint_id_node);
  const meter_type_t type = 1;
  attribute_store_set_reported(meter_type_node, &type, sizeof(type));
  attribute_store_node_t scale_node
    = attribute_store_add_node(ATTRIBUTE(SCALE), meter_type_node);
  const meter_scale_t scale = 2;
  attribute_store_set_reported(scale_node, &scale, sizeof(scale));
  attribute_store_node_t rate_type_node
    = attribute_store_add_node(ATTRIBUTE(RATE_TYPE), scale_node);
  const meter_rate_type_t rate_type = 2;
  attribute_store_set_reported(rate_type_node, &rate_type, sizeof(rate_type));
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(rate_type_node, ATTRIBUTE(VALUE));
  const meter_value_t value = -12345;
  attribute_store_set_desired(value_node, &value, sizeof(value));
  attribute_store_node_t precision_node
    = attribute_store_get_first_child_by_type(rate_type_node,
                                              ATTRIBUTE(PRECISION));
  const meter_precision_t precision = 5;
  attribute_store_set_desired(precision_node, &precision, sizeof(precision));

  TEST_ASSERT_NOT_NULL(meter_reset);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    meter_reset(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_METER_V5,
                                    METER_RESET_V5,
                                    (0 << 7) | (rate_type << 5) | type,
                                    (precision << 5) | (scale << 3) | 4,
                                    (value >> 24) & 0xFF,
                                    (value >> 16) & 0xFF,
                                    (value >> 8) & 0xFF,
                                    value & 0xFF,
                                    0};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_on_meter_reset_complete()
{
  attribute_store_node_t meter_type_node
    = attribute_store_add_node(ATTRIBUTE(TYPE), endpoint_id_node);
  const meter_type_t type = 1;
  attribute_store_set_reported(meter_type_node, &type, sizeof(type));
  attribute_store_node_t scale_node
    = attribute_store_add_node(ATTRIBUTE(SCALE), meter_type_node);
  const meter_scale_t scale = 2;
  attribute_store_set_reported(scale_node, &scale, sizeof(scale));
  attribute_store_node_t rate_type_node
    = attribute_store_add_node(ATTRIBUTE(RATE_TYPE), scale_node);
  const meter_rate_type_t rate_type = 2;
  attribute_store_set_reported(rate_type_node, &rate_type, sizeof(rate_type));
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(rate_type_node, ATTRIBUTE(VALUE));
  const meter_value_t value = -12345;
  attribute_store_set_reported(value_node, &value, sizeof(value));
  attribute_store_set_desired(value_node, &value, sizeof(value));
  attribute_store_node_t precision_node
    = attribute_store_get_first_child_by_type(rate_type_node,
                                              ATTRIBUTE(PRECISION));
  const meter_precision_t precision = 5;
  attribute_store_set_desired(precision_node, &precision, sizeof(precision));

  // Nothing should happen with Get rules:
  TEST_ASSERT_NOT_NULL(on_meter_reset_complete);
  on_meter_reset_complete(value_node,
                          RESOLVER_GET_RULE,
                          FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(value_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(value_node, DESIRED_ATTRIBUTE));

  // Also with Supervision Working, we just wait:
  on_meter_reset_complete(value_node,
                          RESOLVER_SET_RULE,
                          FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(value_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(value_node, DESIRED_ATTRIBUTE));

  // With supervision no support, we go up and mark the reset supported
  // as not supported:
  on_meter_reset_complete(value_node,
                          RESOLVER_SET_RULE,
                          FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT);

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(value_node, REPORTED_ATTRIBUTE));
  attribute_store_node_t reset_support_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(RESET_SUPPORTED));
  meter_reset_supported_t reset_support = true;
  attribute_store_get_reported(reset_support_node,
                               &reset_support,
                               sizeof(reset_support));
  TEST_ASSERT_FALSE(reset_support);

  // Make the supporting node v5:
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_METER_V5),
    endpoint_id_node);
  zwave_cc_version_t version = 5;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  on_meter_reset_complete(value_node,
                          RESOLVER_SET_RULE,
                          FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(value_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(value_node, DESIRED_ATTRIBUTE));

  // Same treatment for v6:
  version = 6;
  attribute_store_set_reported(version_node, &version, sizeof(version));
  attribute_store_set_reported(value_node, &value, sizeof(value));
  attribute_store_set_desired(value_node, &value, sizeof(value));

  on_meter_reset_complete(value_node,
                          RESOLVER_SET_RULE,
                          FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(value_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(value_node, DESIRED_ATTRIBUTE));
}

void test_meter_supported_report_v2_command_happy_case()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_METER, METER_SUPPORTED_REPORT_V5, 0x83, 0x0F};

  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    meter_handler.control_handler(&info,
                                                  incoming_frame,
                                                  sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(4, attribute_store_get_node_child_count(endpoint_id_node));
}

void test_meter_supported_report_v5_command_happy_case()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_METER, METER_SUPPORTED_REPORT_V5, 0x63, 0x8F, 3, 1, 1, 1};

  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    meter_handler.control_handler(&info,
                                                  incoming_frame,
                                                  sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(4, attribute_store_get_node_child_count(endpoint_id_node));
}

void test_meter_supported_report_too_short()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_METER, METER_SUPPORTED_REPORT_V5, 0x83};

  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    meter_handler.control_handler(&info,
                                                  incoming_frame,
                                                  sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_id_node));
}

void test_meter_unknown_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_METER, 0xFF};

  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    meter_handler.control_handler(&info,
                                                  incoming_frame,
                                                  sizeof(incoming_frame)));
}

void test_meter_no_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_METER};

  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    meter_handler.control_handler(&info,
                                                  incoming_frame,
                                                  sizeof(incoming_frame)));
}

void test_meter_wrong_command_class()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_BASIC, METER_REPORT_V5, 1, 2, 3, 4, 5};

  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    meter_handler.control_handler(&info,
                                                  incoming_frame,
                                                  sizeof(incoming_frame)));
}

void test_meter_report_happy_case()
{
  // Simulate a frame from CTT:
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_METER, METER_REPORT_V5, 0x01, 0x42, 0x09, 0x77};

  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    meter_handler.control_handler(&info,
                                                  incoming_frame,
                                                  sizeof(incoming_frame)));
  attribute_store_node_t type_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(TYPE));
  attribute_store_node_t scale_node
    = attribute_store_get_first_child_by_type(type_node, ATTRIBUTE(SCALE));
  attribute_store_node_t rate_type_node
    = attribute_store_get_first_child_by_type(scale_node, ATTRIBUTE(RATE_TYPE));
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(rate_type_node, ATTRIBUTE(VALUE));
  attribute_store_node_t precision_node
    = attribute_store_get_first_child_by_type(rate_type_node,
                                              ATTRIBUTE(PRECISION));

  meter_type_t type = 0;
  attribute_store_get_reported(type_node, &type, sizeof(type));
  TEST_ASSERT_EQUAL(1, type);

  meter_scale_t scale = 0xFFFF;
  attribute_store_get_reported(scale_node, &scale, sizeof(scale));
  TEST_ASSERT_EQUAL(0, scale);

  meter_rate_type_t rate_type = 0xFF;
  attribute_store_get_reported(rate_type_node, &rate_type, sizeof(rate_type));
  TEST_ASSERT_EQUAL(0, rate_type);

  meter_value_t value = 0;
  attribute_store_get_reported(value_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(2423, value);

  meter_precision_t precision = 0;
  attribute_store_get_reported(precision_node, &precision, sizeof(precision));
  TEST_ASSERT_EQUAL(2, precision);
}

void test_meter_report_v1_node_with_advanced_scales()
{
  // Simulate a version 1 node.
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_METER_V5),
    endpoint_id_node);
  const zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Simulate a frame
  const uint8_t incoming_frame[] = {COMMAND_CLASS_METER,
                                    METER_REPORT_V5,
                                    0x80 | (3 << 5) | 6,
                                    ((7 << 5) | (3 << 3) | 0x4),
                                    0x01,
                                    0x01,
                                    0x01,
                                    0x01,
                                    0,
                                    0,
                                    0x2};

  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    meter_handler.control_handler(&info,
                                                  incoming_frame,
                                                  sizeof(incoming_frame)));
  attribute_store_node_t type_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(TYPE));
  attribute_store_node_t scale_node
    = attribute_store_get_first_child_by_type(type_node, ATTRIBUTE(SCALE));
  attribute_store_node_t rate_type_node
    = attribute_store_get_first_child_by_type(scale_node, ATTRIBUTE(RATE_TYPE));
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(rate_type_node, ATTRIBUTE(VALUE));
  attribute_store_node_t precision_node
    = attribute_store_get_first_child_by_type(rate_type_node,
                                              ATTRIBUTE(PRECISION));

  meter_type_t type = 0;
  attribute_store_get_reported(type_node, &type, sizeof(type));
  TEST_ASSERT_EQUAL(6, type);

  meter_scale_t scale = 0xFFFF;
  attribute_store_get_reported(scale_node, &scale, sizeof(scale));
  TEST_ASSERT_EQUAL(10, scale);

  meter_rate_type_t rate_type = 0xFF;
  attribute_store_get_reported(rate_type_node, &rate_type, sizeof(rate_type));
  TEST_ASSERT_EQUAL(3, rate_type);

  meter_value_t value = 0;
  attribute_store_get_reported(value_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(0x01010101, value);

  meter_precision_t precision = 0;
  attribute_store_get_reported(precision_node, &precision, sizeof(precision));
  TEST_ASSERT_EQUAL(7, precision);

  // Check that the supported scales got updated for v1 nodes:
  attribute_store_node_t supported_scales_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SUPPORTED_SCALES));
  uint8_t supported_scales[4] = {};
  attribute_store_get_reported(supported_scales_node,
                               supported_scales,
                               sizeof(supported_scales));
  TEST_ASSERT_EQUAL(0, supported_scales[0]);
  TEST_ASSERT_EQUAL(1 << 2, supported_scales[1]);
  TEST_ASSERT_EQUAL(0, supported_scales[2]);
  TEST_ASSERT_EQUAL(0, supported_scales[3]);
}

void test_meter_report_missing_scale_2_field()
{
  // Simulate a version 1 node.
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_METER_V5),
    endpoint_id_node);
  const zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Simulate a frame
  const uint8_t incoming_frame[] = {COMMAND_CLASS_METER,
                                    METER_REPORT_V5,
                                    0x80 | (3 << 5) | 6,
                                    ((7 << 5) | (3 << 3) | 0x1),
                                    0x01,
                                    1,
                                    0,
                                    0x02};

  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    meter_handler.control_handler(&info,
                                                  incoming_frame,
                                                  sizeof(incoming_frame)));
  attribute_store_node_t type_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(TYPE));

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, type_node);
}

void test_meter_report_too_short()
{
  // Simulate a version 1 node.
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_METER_V5),
    endpoint_id_node);
  const zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Simulate a frame
  const uint8_t incoming_frame[] = {COMMAND_CLASS_METER,
                                    METER_REPORT_V5,
                                    0x80 | (3 << 5) | 6,
                                    ((7 << 5) | (3 << 3) | 0x1)};

  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    meter_handler.control_handler(&info,
                                                  incoming_frame,
                                                  sizeof(incoming_frame)));
  attribute_store_node_t type_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(TYPE));

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, type_node);
}

void test_meter_command_class_no_command()
{
  // Simulate a frame
  const uint8_t incoming_frame[] = {COMMAND_CLASS_METER};

  TEST_ASSERT_NOT_NULL(meter_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    meter_handler.control_handler(&info,
                                                  incoming_frame,
                                                  sizeof(incoming_frame)));
}

void test_supported_rate_types_unknown_values()
{
  const meter_type_t type = 1;
  attribute_store_node_t meter_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(TYPE),
                              &type,
                              sizeof(type));
  meter_scale_t scale = 1;
  attribute_store_node_t scale_node_1
    = attribute_store_emplace(meter_type_node,
                              ATTRIBUTE(SCALE),
                              &scale,
                              sizeof(scale));
  scale = 2;
  attribute_store_node_t scale_node_2
    = attribute_store_emplace(meter_type_node,
                              ATTRIBUTE(SCALE),
                              &scale,
                              sizeof(scale));

  attribute_store_node_t supported_rate_types
    = attribute_store_add_node(ATTRIBUTE(SUPPORTED_RATE_TYPES),
                               endpoint_id_node);
  meter_supported_rate_types_t supported_types = 7;
  attribute_store_set_reported(supported_rate_types,
                               &supported_types,
                               sizeof(supported_types));

  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(scale_node_1));
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(scale_node_2));

  attribute_store_undefine_reported(supported_rate_types);

  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(scale_node_1));
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(scale_node_2));

  supported_types = 1;
  attribute_store_set_reported(supported_rate_types,
                               &supported_types,
                               sizeof(supported_types));

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(scale_node_1));
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(scale_node_2));

  supported_types = 2;
  attribute_store_set_reported(supported_rate_types,
                               &supported_types,
                               sizeof(supported_types));

  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(scale_node_1));
  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(scale_node_2));
}
