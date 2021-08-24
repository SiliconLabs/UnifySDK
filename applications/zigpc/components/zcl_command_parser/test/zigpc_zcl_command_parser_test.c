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

#include "unity.h"
#include "string.h"

#include "zigpc_gateway_notify_mock.h"

#include "zcl_command_parser.h"
#include "zcl_command_parser_int.h"

/**
 * @brief The following test suite uses the ZCL Groups cluster command
 * payloads to verify the functionality of the listeners invoked.
 *
 */

/* variables used by stub mock callbacks */
zigbee_eui64_t stub_cb_eui64;
zigbee_endpoint_id_t stub_cb_endpoint;
zigpc_zclcmdparse_callback_data_t *stub_cb_data;
int stub_cb_num_calls;

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test case
 *
 */
void setUp(void)
{
  zigpc_zclcmdparse_reset_listeners();
  stub_cb_num_calls = 0;
}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void) {}

void stub_add_group_response_cb(const zigbee_eui64_t eui64,
                                const zigbee_endpoint_id_t endpoint,
                                const zigpc_zclcmdparse_callback_data_t *data)
{
  stub_cb_num_calls++;
  TEST_ASSERT_EQUAL_HEX8_ARRAY(stub_cb_eui64, eui64, sizeof(zigbee_eui64_t));
  TEST_ASSERT_EQUAL(stub_cb_endpoint, endpoint);
  TEST_ASSERT_EQUAL_HEX8(stub_cb_data->groups_add_group_response.status,
                         data->groups_add_group_response.status);
  TEST_ASSERT_EQUAL_HEX16(stub_cb_data->groups_add_group_response.group_id,
                          data->groups_add_group_response.group_id);
}

void stub_view_group_response_cb(const zigbee_eui64_t eui64,
                                 const zigbee_endpoint_id_t endpoint,
                                 const zigpc_zclcmdparse_callback_data_t *data)
{
  stub_cb_num_calls++;
  TEST_ASSERT_EQUAL_HEX8_ARRAY(stub_cb_eui64, eui64, sizeof(zigbee_eui64_t));
  TEST_ASSERT_EQUAL(stub_cb_endpoint, endpoint);
  TEST_ASSERT_EQUAL_HEX8(stub_cb_data->groups_view_group_response.status,
                         data->groups_view_group_response.status);
  TEST_ASSERT_EQUAL_HEX16(stub_cb_data->groups_view_group_response.group_id,
                          data->groups_view_group_response.group_id);
  TEST_ASSERT_EQUAL(stub_cb_data->groups_view_group_response.group_name_length,
                    data->groups_view_group_response.group_name_length);
  TEST_ASSERT_EQUAL_STRING_LEN(
    stub_cb_data->groups_view_group_response.group_name,
    data->groups_view_group_response.group_name,
    stub_cb_data->groups_view_group_response.group_name_length);
}

void listener_stub_view_membership_response(
  const zigbee_eui64_t eui64,
  const zigbee_endpoint_id_t endpoint,
  const zigpc_zclcmdparse_callback_data_t *data)
{
  stub_cb_num_calls++;
  TEST_ASSERT_EQUAL_HEX8_ARRAY(stub_cb_eui64, eui64, sizeof(zigbee_eui64_t));
  TEST_ASSERT_EQUAL(stub_cb_endpoint, endpoint);
  TEST_ASSERT_EQUAL_HEX8(
    stub_cb_data->groups_get_group_membership_response.capacity,
    data->groups_get_group_membership_response.capacity);
  TEST_ASSERT_EQUAL_HEX8(
    stub_cb_data->groups_get_group_membership_response.group_list_count,
    data->groups_get_group_membership_response.group_list_count);
  TEST_ASSERT_EQUAL_HEX16_ARRAY(
    stub_cb_data->groups_get_group_membership_response.group_list,
    data->groups_get_group_membership_response.group_list,
    stub_cb_data->groups_get_group_membership_response.group_list_count);
}

/**
 * Callback registration tests
 **/

void test_setup_should_register_gateway_notification(void)
{
  // ARRANGE
  sl_status_t status;

  zigpc_gateway_register_observer_ExpectAndReturn(
    ZIGPC_GATEWAY_NOTIFY_ZCL_COMMAND_RECEIVED,
    NULL,
    SL_STATUS_OK);
  zigpc_gateway_register_observer_IgnoreArg_callback();

  // ACT
  status = zigpc_zclcmdparse_setup();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

void test_unknown_type_callback_listeners(void)
{
  // ARRANGE
  sl_status_t status_register;
  sl_status_t status_remove;

  // ACT
  status_register
    = zigpc_zclcmdparse_register_callback(0xFFFF,
                                          0xFF,
                                          stub_add_group_response_cb);
  status_remove
    = zigpc_zclcmdparse_register_callback(0xFFFF,
                                          0xFF,
                                          stub_add_group_response_cb);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_TYPE, status_register);
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_TYPE, status_remove);
}

void test_register_multiple_listeners_with_known_types(void)
{
  // ARRANGE
  sl_status_t status_listener_1;
  sl_status_t status_listener_2;

  // ACT
  status_listener_1 = zigpc_zclcmdparse_register_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_ADD_GROUP_RESPONSE,
    stub_add_group_response_cb);
  status_listener_2 = zigpc_zclcmdparse_register_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_GET_GROUP_MEMBERSHIP_RESPONSE,
    stub_view_group_response_cb);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status_listener_1);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status_listener_2);

  // ACT
  status_listener_1 = zigpc_zclcmdparse_remove_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_ADD_GROUP_RESPONSE,
    stub_add_group_response_cb);
  status_listener_2 = zigpc_zclcmdparse_remove_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_GET_GROUP_MEMBERSHIP_RESPONSE,
    stub_view_group_response_cb);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status_listener_1);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status_listener_2);
}

void test_listener_not_invoked_when_parsing_server_cluster(void)
{
  // ARRANGE
  zigpc_gateway_on_command_received_t test_event = {
    .from_server_to_client = false,
  };

  zigpc_zclcmdparse_on_command_received(&test_event);

  // ASSERT
  TEST_ASSERT_EQUAL(ZIGPC_ZCL_STATUS_UNSUPPORTED_CLUSTER,
                    test_event.return_status);
}

void test_listener_not_invoked_when_parsing_unsupported_cluster(void)
{
  // ARRANGE
  zigpc_gateway_on_command_received_t test_event = {
    .cluster_id            = 0xFFFF,
    .from_server_to_client = true,
  };

  zigpc_zclcmdparse_on_command_received(&test_event);

  // ASSERT
  TEST_ASSERT_EQUAL(ZIGPC_ZCL_STATUS_UNSUPPORTED_CLUSTER,
                    test_event.return_status);
}

void test_listener_not_invoked_when_parsing_unsupported_command(void)
{
  // ARRANGE
  zigpc_gateway_on_command_received_t test_event = {
    .cluster_id            = 0x0004,
    .command_id            = 0xFF,
    .from_server_to_client = true,
  };

  zigpc_zclcmdparse_on_command_received(&test_event);

  // ASSERT
  TEST_ASSERT_EQUAL(ZIGPC_ZCL_STATUS_UNSUP_CLUSTER_COMMAND,
                    test_event.return_status);
}

void test_listener_not_invoked_when_parsing_malformed_short_command(void)
{
  // ARRANGE
  sl_status_t registration_status;
  zigpc_gateway_on_command_received_t test_event
    = {.cluster_id            = 0x0004,
       .command_id            = 0x0000,
       .from_server_to_client = true,
       .frame_payload_offset  = 0,
       .return_status         = -1,
       .frame                 = {.size = 2, .buffer = "\x00\x34"}};

  // ACT
  registration_status = zigpc_zclcmdparse_register_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_ADD_GROUP_RESPONSE,
    stub_add_group_response_cb);
  zigpc_zclcmdparse_on_command_received(&test_event);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, registration_status);
  TEST_ASSERT_EQUAL(ZIGPC_ZCL_STATUS_MALFORMED_COMMAND,
                    test_event.return_status);
  TEST_ASSERT_EQUAL(0, stub_cb_num_calls);
}

void test_listener_invoked_when_parsing_command_with_primitive_data_types(void)
{
  // ARRANGE
  sl_status_t registration_status;
  zigpc_gateway_on_command_received_t test_event
    = {.cluster_id            = 0x0004,
       .command_id            = 0x0000,
       .from_server_to_client = true,
       .frame_payload_offset  = 0,
       .return_status         = -1,
       .frame                 = {.size = 3, .buffer = "\x00\x34\x12"}};
  zigpc_zclcmdparse_callback_data_t expected_data
    = {.groups_add_group_response = {
         .status   = ZIGPC_ZCL_STATUS_SUCCESS,
         .group_id = 0x1234,
       }};

  memcpy(stub_cb_eui64, test_event.eui64, sizeof(zigbee_eui64_t));
  stub_cb_endpoint = test_event.endpoint_id;
  stub_cb_data     = &expected_data;

  // ACT
  registration_status = zigpc_zclcmdparse_register_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_ADD_GROUP_RESPONSE,
    stub_add_group_response_cb);
  zigpc_zclcmdparse_on_command_received(&test_event);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, registration_status);
  TEST_ASSERT_EQUAL(ZIGPC_ZCL_STATUS_SUCCESS, test_event.return_status);
  TEST_ASSERT_EQUAL(1, stub_cb_num_calls);
}

void test_listener_invoked_when_parsing_command_with_primitive_data_types_non_zero_offset(
  void)
{
  // ARRANGE
  sl_status_t registration_status;
  zigpc_gateway_on_command_received_t test_event = {
    .cluster_id            = 0x0004,
    .command_id            = 0x00,
    .from_server_to_client = true,
    .frame_payload_offset  = 3,
    .return_status         = -1,
    .frame                 = {.size = 6, .buffer = "\xFF\xFF\xFF\x00\x34\x12"},
  };
  zigpc_zclcmdparse_callback_data_t expected_data
    = {.groups_add_group_response = {
         .status   = ZIGPC_ZCL_STATUS_SUCCESS,
         .group_id = 0x1234,
       }};

  memcpy(stub_cb_eui64, test_event.eui64, sizeof(zigbee_eui64_t));
  stub_cb_endpoint = test_event.endpoint_id;
  stub_cb_data     = &expected_data;

  // ACT
  registration_status = zigpc_zclcmdparse_register_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_ADD_GROUP_RESPONSE,
    stub_add_group_response_cb);
  zigpc_zclcmdparse_on_command_received(&test_event);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, registration_status);
  TEST_ASSERT_EQUAL(ZIGPC_ZCL_STATUS_SUCCESS, test_event.return_status);
  TEST_ASSERT_EQUAL(1, stub_cb_num_calls);
}

void test_listener_invoked_when_parsing_command_with_string_data_type(void)
{
  // ARRANGE
  sl_status_t registration_status;
  zigpc_gateway_on_command_received_t test_event
    = {.cluster_id            = 0x0004,
       .command_id            = 0x01,
       .from_server_to_client = true,
       .frame_payload_offset  = 0,
       .return_status         = -1,
       .frame = {.size = 16, .buffer = "\x00\x34\x12\x0BLiving Room"}};
  const char *exp_group_name = "Living Room";
  zigpc_zclcmdparse_callback_data_t expected_data
    = {.groups_view_group_response = {
         .status            = ZIGPC_ZCL_STATUS_SUCCESS,
         .group_id          = 0x1234,
         .group_name_length = 11,
         .group_name        = exp_group_name,
       }};

  memcpy(stub_cb_eui64, test_event.eui64, sizeof(zigbee_eui64_t));
  stub_cb_endpoint = test_event.endpoint_id;
  stub_cb_data     = &expected_data;

  // ACT
  registration_status = zigpc_zclcmdparse_register_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_VIEW_GROUP_RESPONSE,
    stub_view_group_response_cb);
  zigpc_zclcmdparse_on_command_received(&test_event);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, registration_status);
  TEST_ASSERT_EQUAL(ZIGPC_ZCL_STATUS_SUCCESS, test_event.return_status);
  TEST_ASSERT_EQUAL(1, stub_cb_num_calls);
}

void test_listener_invoked_when_parsing_command_with_array_data_type(void)
{
  // ARRANGE
  sl_status_t registration_status;
  zigpc_gateway_on_command_received_t test_event
    = {.cluster_id            = 0x0004,
       .command_id            = 0x02,
       .from_server_to_client = true,
       .frame_payload_offset  = 0,
       .return_status         = -1,
       .frame = {.size = 8, .buffer = "\x0A\x03\x34\x12\x00\x01\x00\x10"}};
  zigpc_zclcmdparse_callback_data_t expected_data
    = {.groups_get_group_membership_response = {
         .capacity         = 10,
         .group_list_count = 3,
         .group_list       = (uint16_t *)&test_event.frame.buffer[2],
       }};

  memcpy(stub_cb_eui64, test_event.eui64, sizeof(zigbee_eui64_t));
  stub_cb_endpoint  = test_event.endpoint_id;
  stub_cb_data      = &expected_data;
  stub_cb_num_calls = 1;

  // ACT
  registration_status = zigpc_zclcmdparse_register_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_VIEW_GROUP_RESPONSE,
    listener_stub_view_membership_response);
  zigpc_zclcmdparse_on_command_received(&test_event);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, registration_status);
  TEST_ASSERT_EQUAL(ZIGPC_ZCL_STATUS_SUCCESS, test_event.return_status);

  zigpc_zclcmdparse_remove_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_VIEW_GROUP_RESPONSE,
    stub_view_group_response_cb);
}
