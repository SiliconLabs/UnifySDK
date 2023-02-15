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
#include "zwave_command_class_security_0.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"
#include "zpc_attribute_store_type_registration.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mocks
#include "zwave_command_handler_mock.h"
#include "zwave_tx_scheme_selector_mock.h"

static zwave_command_handler_t s0_handler = {};
static uint8_t received_frame[255]        = {};
static uint16_t received_frame_size       = 0;

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  s0_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    s0_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY, s0_handler.command_class);
  TEST_ASSERT_EQUAL(SECURITY_VERSION, s0_handler.version);
  TEST_ASSERT_NOT_NULL(s0_handler.control_handler);
  TEST_ASSERT_NOT_NULL(s0_handler.support_handler);
  TEST_ASSERT_NOT_NULL(s0_handler.control_handler);
  TEST_ASSERT_TRUE(s0_handler.manual_security_validation);

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

static void zwave_command_class_s0_init_verification()
{
  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_security_0_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;
  memset(&s0_handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_s0_init_verification();
}

void test_s0_supported_get()
{
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_security_0_commands_supported_get(
                      received_frame,
                      &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_s0_control_handler_command_too_short()
{
  TEST_ASSERT_NOT_NULL(s0_handler.control_handler);
  zwave_controller_connection_info_t info = {};
  const uint8_t incoming_frame[]          = {COMMAND_CLASS_SECURITY};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    s0_handler.control_handler(&info, incoming_frame, sizeof(incoming_frame)));
}

void test_s0_control_handler_command_unknown()
{
  TEST_ASSERT_NOT_NULL(s0_handler.control_handler);
  zwave_controller_connection_info_t info = {};
  const uint8_t incoming_frame[]          = {COMMAND_CLASS_SECURITY, 0xFF};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    s0_handler.control_handler(&info, incoming_frame, sizeof(incoming_frame)));
}

void test_s0_control_handler_supported_level_on_too_low_level()
{
  TEST_ASSERT_NOT_NULL(s0_handler.control_handler);
  zwave_controller_connection_info_t info = {.encapsulation = 0};
  attribute_store_get_reported(node_id_node,
                               &(info.remote.node_id),
                               sizeof(info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(info.remote.endpoint_id),
                               sizeof(info.remote.endpoint_id));
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_REPORT};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    s0_handler.control_handler(&info, incoming_frame, sizeof(incoming_frame)));
}

void test_s0_control_handler_happy_case()
{
  TEST_ASSERT_NOT_NULL(s0_handler.control_handler);
  zwave_controller_connection_info_t info
    = {.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0};
  attribute_store_get_reported(node_id_node,
                               &(info.remote.node_id),
                               sizeof(info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(info.remote.endpoint_id),
                               sizeof(info.remote.endpoint_id));

  const uint8_t incoming_frame[] = {COMMAND_CLASS_SECURITY,
                                    SECURITY_COMMANDS_SUPPORTED_REPORT,
                                    0,
                                    23,
                                    34,
                                    45};

  zwave_tx_scheme_get_node_highest_security_class_ExpectAndReturn(
    info.remote.node_id,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    s0_handler.control_handler(&info, incoming_frame, sizeof(incoming_frame)));

  attribute_store_node_t secure_nif_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);

  const uint8_t expected_data[] = {23, 34, 45};
  uint8_t array_size            = 0;
  attribute_store_get_node_attribute_value(secure_nif_node,
                                           REPORTED_ATTRIBUTE,
                                           received_frame,
                                           &array_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_data, received_frame, array_size);
}

void test_s0_control_handler_wrong_encapsulation_level()
{
  TEST_ASSERT_NOT_NULL(s0_handler.control_handler);
  zwave_controller_connection_info_t info
    = {.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0};
  attribute_store_get_reported(node_id_node,
                               &(info.remote.node_id),
                               sizeof(info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(info.remote.endpoint_id),
                               sizeof(info.remote.endpoint_id));

  const uint8_t incoming_frame[] = {COMMAND_CLASS_SECURITY,
                                    SECURITY_COMMANDS_SUPPORTED_REPORT,
                                    0,
                                    23,
                                    34,
                                    45};

  zwave_tx_scheme_get_node_highest_security_class_ExpectAndReturn(
    info.remote.node_id,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    s0_handler.control_handler(&info, incoming_frame, sizeof(incoming_frame)));

  attribute_store_node_t secure_nif_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);

  uint8_t array_size = 0;
  attribute_store_get_node_attribute_value(secure_nif_node,
                                           REPORTED_ATTRIBUTE,
                                           received_frame,
                                           &array_size);
  TEST_ASSERT_EQUAL(0, array_size);
}

void test_s0_control_handler_report_too_short()
{
  TEST_ASSERT_NOT_NULL(s0_handler.control_handler);
  zwave_controller_connection_info_t info
    = {.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0};
  attribute_store_get_reported(node_id_node,
                               &(info.remote.node_id),
                               sizeof(info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(info.remote.endpoint_id),
                               sizeof(info.remote.endpoint_id));

  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_REPORT, 0};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    s0_handler.control_handler(&info, incoming_frame, sizeof(incoming_frame)));

  attribute_store_node_t secure_nif_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);

  uint8_t array_size = 0;
  attribute_store_get_node_attribute_value(secure_nif_node,
                                           REPORTED_ATTRIBUTE,
                                           received_frame,
                                           &array_size);
  TEST_ASSERT_EQUAL(0, array_size);
}

void test_s0_support_handler_too_short()
{
  TEST_ASSERT_NOT_NULL(s0_handler.support_handler);
  zwave_controller_connection_info_t info = {};

  const uint8_t incoming_frame[] = {COMMAND_CLASS_SECURITY};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    s0_handler.support_handler(&info, incoming_frame, sizeof(incoming_frame)));
}

void test_s0_support_handler_non_supported_command()
{
  TEST_ASSERT_NOT_NULL(s0_handler.support_handler);
  zwave_controller_connection_info_t info = {};

  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_REPORT};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    s0_handler.support_handler(&info, incoming_frame, sizeof(incoming_frame)));
}

void test_s0_control_handler_happy_case_more_reports()
{
  TEST_ASSERT_NOT_NULL(s0_handler.control_handler);
  zwave_controller_connection_info_t info
    = {.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0};
  attribute_store_get_reported(node_id_node,
                               &(info.remote.node_id),
                               sizeof(info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(info.remote.endpoint_id),
                               sizeof(info.remote.endpoint_id));

  const uint8_t incoming_frame[] = {COMMAND_CLASS_SECURITY,
                                    SECURITY_COMMANDS_SUPPORTED_REPORT,
                                    1,
                                    23,
                                    34,
                                    45};

  zwave_tx_scheme_get_node_highest_security_class_ExpectAndReturn(
    info.remote.node_id,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    s0_handler.control_handler(&info, incoming_frame, sizeof(incoming_frame)));

  attribute_store_node_t secure_nif_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);

  const uint8_t expected_data[] = {23, 34, 45};
  uint8_t array_size            = 0;
  attribute_store_get_node_attribute_value(secure_nif_node,
                                           REPORTED_ATTRIBUTE,
                                           received_frame,
                                           &array_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_data, received_frame, array_size);

  const uint8_t new_incoming_frame[] = {COMMAND_CLASS_SECURITY,
                                        SECURITY_COMMANDS_SUPPORTED_REPORT,
                                        0,
                                        24,
                                        35,
                                        46};

  zwave_tx_scheme_get_node_highest_security_class_ExpectAndReturn(
    info.remote.node_id,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    s0_handler.control_handler(&info,
                                               new_incoming_frame,
                                               sizeof(new_incoming_frame)));
  secure_nif_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);

  const uint8_t new_expected_data[] = {23, 34, 45, 24, 35, 46};
  array_size                        = 0;
  attribute_store_get_node_attribute_value(secure_nif_node,
                                           REPORTED_ATTRIBUTE,
                                           received_frame,
                                           &array_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(new_expected_data, received_frame, array_size);
}