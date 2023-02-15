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

// Includes from other components
#include "unity.h"
#include "string.h"
#include "zwapi_init_test.h"

// Includes from this component
#include "zwapi_init.h"

#include "zwapi_func_ids.h"
#include "zwapi_session_mock.h"
#include "zwapi_protocol_basis_mock.h"

// Static variables
static uint8_t response_length      = 0;
static uint8_t response_buffer[255] = {0};

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

/// Test of zwapi_get_protocol_version
void test_zwapi_get_protocol_version(void)
{
  zwapi_protocol_version_information_t received_protocol = {0};
  memcpy(response_buffer,
         zwapi_get_protocol_version_response,
         sizeof(zwapi_get_protocol_version_response));

  // First off get a failure from send_frame, we should return fail
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_ZW_GET_PROTOCOL_VERSION,
    NULL,
    0,
    NULL,
    NULL,
    SL_STATUS_TRANSMIT_UNDERFLOW);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwapi_get_protocol_version(&received_protocol));
  TEST_ASSERT_EQUAL(0, received_protocol.type);
  TEST_ASSERT_EQUAL(0, received_protocol.major_version);
  TEST_ASSERT_EQUAL(0, received_protocol.minor_version);
  TEST_ASSERT_EQUAL(0, received_protocol.revision_version);
  TEST_ASSERT_EQUAL(0, received_protocol.build_number);
  TEST_ASSERT_EQUAL(0, received_protocol.git_commit[0]);
  TEST_ASSERT_EQUAL(0, received_protocol.git_commit[4]);

  // Receive a frame without payload
  response_length = 0 + IDX_DATA;
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_ZW_GET_PROTOCOL_VERSION,
    NULL,
    0,
    NULL,
    NULL,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwapi_get_protocol_version(&received_protocol));
  TEST_ASSERT_EQUAL(0, received_protocol.type);
  TEST_ASSERT_EQUAL(0, received_protocol.major_version);
  TEST_ASSERT_EQUAL(0, received_protocol.minor_version);
  TEST_ASSERT_EQUAL(0, received_protocol.revision_version);
  TEST_ASSERT_EQUAL(0, received_protocol.build_number);
  TEST_ASSERT_EQUAL(0, received_protocol.git_commit[0]);
  TEST_ASSERT_EQUAL(0, received_protocol.git_commit[4]);

  // Receive a frame too short, missing the protocol version (major minor revision)
  response_length = 3 + IDX_DATA;
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_ZW_GET_PROTOCOL_VERSION,
    NULL,
    0,
    NULL,
    NULL,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwapi_get_protocol_version(&received_protocol));
  TEST_ASSERT_EQUAL(0xB2, received_protocol.type);
  TEST_ASSERT_EQUAL(0, received_protocol.major_version);
  TEST_ASSERT_EQUAL(0, received_protocol.minor_version);
  TEST_ASSERT_EQUAL(0, received_protocol.revision_version);
  TEST_ASSERT_EQUAL(0, received_protocol.build_number);
  TEST_ASSERT_EQUAL(0, received_protocol.git_commit[0]);
  TEST_ASSERT_EQUAL(0, received_protocol.git_commit[4]);

  // Receive a frame too short, missing the build number
  response_length = 5 + IDX_DATA;
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_ZW_GET_PROTOCOL_VERSION,
    NULL,
    0,
    NULL,
    NULL,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwapi_get_protocol_version(&received_protocol));
  TEST_ASSERT_EQUAL(0xB2, received_protocol.type);
  TEST_ASSERT_EQUAL(0x53, received_protocol.major_version);
  TEST_ASSERT_EQUAL(0x0f, received_protocol.minor_version);
  TEST_ASSERT_EQUAL(0x01, received_protocol.revision_version);
  TEST_ASSERT_EQUAL(0, received_protocol.build_number);
  TEST_ASSERT_EQUAL(0, received_protocol.git_commit[0]);
  TEST_ASSERT_EQUAL(0, received_protocol.git_commit[4]);

  // Receive a frame too short, missing the git commit fields
  response_length = 20 + IDX_DATA;
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_ZW_GET_PROTOCOL_VERSION,
    NULL,
    0,
    NULL,
    NULL,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwapi_get_protocol_version(&received_protocol));
  TEST_ASSERT_EQUAL(0xB2, received_protocol.type);
  TEST_ASSERT_EQUAL(0x53, received_protocol.major_version);
  TEST_ASSERT_EQUAL(0x0f, received_protocol.minor_version);
  TEST_ASSERT_EQUAL(0x01, received_protocol.revision_version);
  TEST_ASSERT_EQUAL(0x7E07, received_protocol.build_number);
  TEST_ASSERT_EQUAL(0, received_protocol.git_commit[0]);
  TEST_ASSERT_EQUAL(0, received_protocol.git_commit[4]);

  // Happy case: we receive everything, and even more than needed
  response_length
    = sizeof(zwapi_get_protocol_version_response) / sizeof(uint8_t);
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_ZW_GET_PROTOCOL_VERSION,
    NULL,
    0,
    NULL,
    NULL,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwapi_get_protocol_version(&received_protocol));
  TEST_ASSERT_EQUAL(0xB2, received_protocol.type);
  TEST_ASSERT_EQUAL(0x53, received_protocol.major_version);
  TEST_ASSERT_EQUAL(0x0f, received_protocol.minor_version);
  TEST_ASSERT_EQUAL(0x01, received_protocol.revision_version);
  TEST_ASSERT_EQUAL(0x7E07, received_protocol.build_number);
  TEST_ASSERT_EQUAL(0x30, received_protocol.git_commit[0]);
  TEST_ASSERT_EQUAL(0x34, received_protocol.git_commit[4]);
  TEST_ASSERT_EQUAL(0x46, received_protocol.git_commit[15]);
}

void test_zwapi_awaiting_zwave_api_started(void)
{
  zwapi_set_awaiting_zwave_api_started(false);
  TEST_ASSERT_FALSE(zwapi_is_awaiting_zwave_api_started());
  zwapi_set_awaiting_zwave_api_started(true);
  TEST_ASSERT_TRUE(zwapi_is_awaiting_zwave_api_started());
}

void test_zwapi_init()
{
  int serial_fd               = 0;
  zwapi_callbacks_t callbacks = {};

  zwapi_session_init_ExpectAndReturn("test", 3);
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_SERIAL_API_GET_CAPABILITIES,
    NULL,
    0,
    NULL,
    NULL,
    SL_STATUS_FAIL);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();

  zwapi_init("test", &serial_fd, &callbacks);
  TEST_ASSERT_EQUAL(3, serial_fd);
}

void test_zwapi_refresh_capabilities_bad_cases()
{
  //Capablities status fail
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_SERIAL_API_GET_CAPABILITIES,
    NULL,
    0,
    NULL,
    NULL,
    SL_STATUS_FAIL);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwapi_refresh_capabilities());

  uint8_t response_length = 10;

  //Capablities status ok but response length do not have Z-Wave module data
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_SERIAL_API_GET_CAPABILITIES,
    NULL,
    0,
    NULL,
    NULL,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwapi_refresh_capabilities());
}

void test_zwapi_get_init_data()
{
  //Happy case
  uint8_t res_buffer[]
    = {0x19, 0x01, 0x09, 0xb2, 0x53, 0x02, 0x01, 0x02, 0x07, 0x30};
  response_length = 10;

  uint8_t ver, capabilities, len, chip_type, chip_version;
  uint8_t node_list[501] = {0};
  ver                  = 1;
  capabilities         = 1;
  len                  = 1;
  chip_type            = 1;
  chip_version         = 1;
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_SERIAL_API_GET_INIT_DATA,
    NULL,
    0,
    res_buffer,
    &response_length,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    res_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwapi_get_init_data(&ver,
                                        &capabilities,
                                        &len,
                                        node_list,
                                        &chip_type,
                                        &chip_version));
  uint8_t list[501] = {0x01, 0x02};
  TEST_ASSERT_EQUAL(0xB2, ver);
  TEST_ASSERT_EQUAL(0x53, capabilities);
  TEST_ASSERT_EQUAL(0x02, len);
  TEST_ASSERT_EQUAL_INT8_ARRAY(list, node_list, 2);
  TEST_ASSERT_EQUAL(0x07, chip_type);
  TEST_ASSERT_EQUAL(0x30, chip_version);

  //Send command status fail
  zwapi_session_send_frame_with_response_IgnoreAndReturn(SL_STATUS_FAIL);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwapi_get_init_data(&ver,
                                        &capabilities,
                                        &len,
                                        node_list,
                                        &chip_type,
                                        &chip_version));

  //Response length smaller than IDX_DATA
  response_length = 2;
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_SERIAL_API_GET_INIT_DATA,
    NULL,
    0,
    res_buffer,
    &response_length,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    res_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwapi_get_init_data(&ver,
                                        &capabilities,
                                        &len,
                                        node_list,
                                        &chip_type,
                                        &chip_version));
}
void test_zwapi_support_setup_func(void)
{
  uint16_t cmd;
  cmd = 0 << 8;
  cmd |= 9;
  TEST_ASSERT_EQUAL(false, zwapi_support_setup_func(cmd));
}
void test_zwapi_log_to_file_enable(void)
{
  char *filename = "";
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwapi_log_to_file_enable(filename));
}

void test_zwapi_log_to_file_disable(void)
{
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwapi_log_to_file_disable());
}
