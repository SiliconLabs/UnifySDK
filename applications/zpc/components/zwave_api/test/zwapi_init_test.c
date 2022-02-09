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