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
#include "multicast_follow_ups_transport.h"

// Other components
#include "zwave_tx_groups.h"
#include "zwave_node_id_definitions.h"

// Unity test
#include "unity.h"

// Generic includes
#include <string.h>

// Mock includes
#include "zwave_controller_transport_mock.h"
#include "zwave_controller_internal_mock.h"
#include "zwave_tx_mock.h"

// Static test variables
static zwave_controller_transport_t multicast_transport;
static on_zwave_tx_send_data_complete_t multicast_transport_send_data_callback;

static zwave_controller_connection_info_t info;
static zwave_tx_options_t tx_options;
static zwave_tx_session_id_t parent_session_id;
static void *user;

static uint8_t received_status;
static zwapi_tx_report_t received_tx_report;
static void *received_user;
static int send_done_count;
static sl_status_t tx_send_data_return_value = SL_STATUS_OK;

// Sub functions
static sl_status_t zwave_controller_transport_register_stub(
  const zwave_controller_transport_t *transport, int cmock_num_calls)
{
  TEST_ASSERT_NOT_NULL(transport);
  multicast_transport = *transport;

  TEST_ASSERT_NOT_NULL(multicast_transport.abort_send_data);
  TEST_ASSERT_NOT_NULL(multicast_transport.send_data);
  TEST_ASSERT_NULL(multicast_transport.on_frame_received);
  TEST_ASSERT_EQUAL(0x00, multicast_transport.command_class);
  TEST_ASSERT_EQUAL(0, multicast_transport.version);
  TEST_ASSERT_EQUAL(6, multicast_transport.priority);
  return SL_STATUS_OK;
}

static sl_status_t zwave_tx_send_data_stub(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session,
  int cmock_num_calls)
{
  multicast_transport_send_data_callback = on_send_complete;
  return tx_send_data_return_value;
}

// test callbacks
static void on_send_data_complete(uint8_t status,
                                  const zwapi_tx_report_t *tx_report,
                                  void *user)
{
  send_done_count += 1;
  received_status = status;
  if (tx_report != NULL) {
    received_tx_report = *tx_report;
  }
  received_user = user;
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
  memset(&multicast_transport, 0, sizeof(multicast_transport));
  memset(&info, 0, sizeof(info));
  memset(&tx_options, 0, sizeof(tx_options));
  user              = NULL;
  parent_session_id = NULL;

  received_status = 0xFF;
  memset(&received_tx_report, 0, sizeof(received_tx_report));
  received_user                          = NULL;
  send_done_count                        = 0;
  tx_send_data_return_value              = SL_STATUS_OK;
  multicast_transport_send_data_callback = NULL;
  zwave_controller_transport_register_Stub(
    zwave_controller_transport_register_stub);

  zwave_tx_send_data_Stub(&zwave_tx_send_data_stub);

  zwave_tx_get_number_of_responses_IgnoreAndReturn(1);
  zwave_controller_on_multicast_group_deleted_Ignore();

  TEST_ASSERT_EQUAL(SL_STATUS_OK, multicast_follow_ups_transport_init());
}

void test_zwave_multicast_transport_send_data_not_supported()
{
  TEST_ASSERT_NOT_NULL(multicast_transport.send_data);

  info.remote.multicast_group = 1;
  user                        = (void *)34;
  parent_session_id           = (void *)53;
  const uint8_t frame[]       = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    multicast_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  // Both is_multicast and send_follow_ups needs to be true to activate the transport
  info.remote.is_multicast = true;
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    multicast_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  info.remote.is_multicast   = false;
  tx_options.send_follow_ups = true;
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    multicast_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));
}

void test_zwave_multicast_transport_send_data_happy_case_1_node()
{
  TEST_ASSERT_NOT_NULL(multicast_transport.send_data);

  // Prepare data, only 1 node in the group:
  zwave_nodemask_t node_list          = {0};
  zwave_multicast_group_id_t group_id = ZWAVE_TX_INVALID_GROUP;
  ZW_ADD_NODE_TO_MASK(1, node_list);
  zwave_tx_assign_group(node_list, &group_id);

  info.remote.multicast_group = group_id;
  info.remote.is_multicast    = true;
  tx_options.send_follow_ups  = true;
  user                        = (void *)34;
  parent_session_id           = (void *)53;
  const uint8_t frame[]       = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multicast_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  // Make a callback, expect 1 follow-up to get queued. It will read the frame
  // data from the previous multicast frame.
  zwave_tx_get_frame_length_IgnoreAndReturn(sizeof(frame));
  zwave_tx_get_frame_IgnoreAndReturn(frame);
  TEST_ASSERT_NOT_NULL(multicast_transport_send_data_callback);
  multicast_transport_send_data_callback(TRANSMIT_COMPLETE_OK, NULL, NULL);

  TEST_ASSERT_EQUAL(0, send_done_count);

  TEST_ASSERT_NOT_NULL(multicast_transport_send_data_callback);
  multicast_transport_send_data_callback(TRANSMIT_COMPLETE_OK, NULL, NULL);

  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, received_status);
}

void test_zwave_multicast_transport_send_data_happy_case_4_nodes()
{
  TEST_ASSERT_NOT_NULL(multicast_transport.send_data);

  // Prepare data, only 1 node in the group:
  zwave_nodemask_t node_list          = {0};
  zwave_multicast_group_id_t group_id = ZWAVE_TX_INVALID_GROUP;
  ZW_ADD_NODE_TO_MASK(10, node_list);
  ZW_ADD_NODE_TO_MASK(2, node_list);
  ZW_ADD_NODE_TO_MASK(3, node_list);
  ZW_ADD_NODE_TO_MASK(4, node_list);
  zwave_tx_assign_group(node_list, &group_id);

  info.remote.multicast_group = group_id;
  info.remote.is_multicast    = true;
  tx_options.send_follow_ups  = true;
  user                        = (void *)34;
  parent_session_id           = (void *)53;
  const uint8_t frame[]       = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multicast_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  // Double call won't work, we are busy!
  TEST_ASSERT_EQUAL(SL_STATUS_BUSY,
                    multicast_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  // Make a callback, expect 1 follow-up to get queued. It will read the frame
  // data from the previous multicast frame.
  // Node 2
  zwave_tx_get_frame_length_IgnoreAndReturn(sizeof(frame));
  zwave_tx_get_frame_IgnoreAndReturn(frame);
  TEST_ASSERT_NOT_NULL(multicast_transport_send_data_callback);
  multicast_transport_send_data_callback(TRANSMIT_COMPLETE_OK, NULL, NULL);

  TEST_ASSERT_EQUAL(0, send_done_count);

  // Node 3
  zwave_tx_get_frame_length_IgnoreAndReturn(sizeof(frame));
  zwave_tx_get_frame_IgnoreAndReturn(frame);
  multicast_transport_send_data_callback(TRANSMIT_COMPLETE_OK, NULL, NULL);

  TEST_ASSERT_EQUAL(0, send_done_count);

  // Node 4
  zwave_tx_get_frame_length_IgnoreAndReturn(sizeof(frame));
  zwave_tx_get_frame_IgnoreAndReturn(frame);
  multicast_transport_send_data_callback(TRANSMIT_COMPLETE_OK, NULL, NULL);

  TEST_ASSERT_EQUAL(0, send_done_count);

  // Node 10
  zwave_tx_get_frame_length_IgnoreAndReturn(sizeof(frame));
  zwave_tx_get_frame_IgnoreAndReturn(frame);
  multicast_transport_send_data_callback(TRANSMIT_COMPLETE_OK, NULL, NULL);

  TEST_ASSERT_EQUAL(0, send_done_count);

  // Now we are done!
  multicast_transport_send_data_callback(TRANSMIT_COMPLETE_FAIL, NULL, NULL);

  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, received_status);
}

void test_zwave_multicast_transport_abort_send_data()
{
  TEST_ASSERT_NOT_NULL(multicast_transport.send_data);
  TEST_ASSERT_NOT_NULL(multicast_transport.abort_send_data);

  // Prepare data, only 1 node in the group:
  zwave_nodemask_t node_list          = {0};
  zwave_multicast_group_id_t group_id = ZWAVE_TX_INVALID_GROUP;
  ZW_ADD_NODE_TO_MASK(1, node_list);
  ZW_ADD_NODE_TO_MASK(2, node_list);
  zwave_tx_assign_group(node_list, &group_id);

  info.remote.multicast_group = group_id;
  info.remote.is_multicast    = true;
  tx_options.send_follow_ups  = true;
  user                        = (void *)34;
  parent_session_id           = (void *)53;
  const uint8_t frame[]       = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multicast_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  TEST_ASSERT_EQUAL(0, send_done_count);

  // Abort wrong session, should not do anything.
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    multicast_transport.abort_send_data(parent_session_id + 1));
  TEST_ASSERT_EQUAL(0, send_done_count);

  // Abort right session, now it triggers a callback:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multicast_transport.abort_send_data(parent_session_id));

  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, received_status);

  // same sesison cannot be aborted twice
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    multicast_transport.abort_send_data(parent_session_id));

  // Requeueing new stuff should work:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multicast_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));
}

void test_zwave_multicast_transport_callback_with_no_session_ongoing()
{
  TEST_ASSERT_NOT_NULL(multicast_transport.send_data);

  // Prepare data, only 1 node in the group:
  zwave_nodemask_t node_list          = {0};
  zwave_multicast_group_id_t group_id = ZWAVE_TX_INVALID_GROUP;
  ZW_ADD_NODE_TO_MASK(1, node_list);
  zwave_tx_assign_group(node_list, &group_id);

  info.remote.multicast_group = group_id;
  info.remote.is_multicast    = true;
  tx_options.send_follow_ups  = true;
  user                        = (void *)34;
  parent_session_id           = (void *)53;
  const uint8_t frame[]       = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multicast_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  TEST_ASSERT_EQUAL(0, send_done_count);

  // Abort right session, now it triggers a callback:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multicast_transport.abort_send_data(parent_session_id));

  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, received_status);

  // Try to make a send data callback now, nothing bad shoudl happen:
  multicast_transport_send_data_callback(TRANSMIT_COMPLETE_OK, NULL, NULL);
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, received_status);
}

void test_zwave_multicast_transport_tx_queue_rejects()
{
  TEST_ASSERT_NOT_NULL(multicast_transport.send_data);

  // Prepare data, only 1 node in the group:
  zwave_nodemask_t node_list          = {0};
  zwave_multicast_group_id_t group_id = ZWAVE_TX_INVALID_GROUP;
  ZW_ADD_NODE_TO_MASK(1, node_list);
  zwave_tx_assign_group(node_list, &group_id);

  info.remote.multicast_group = group_id;
  info.remote.is_multicast    = true;
  tx_options.send_follow_ups  = true;
  user                        = (void *)34;
  parent_session_id           = (void *)53;
  const uint8_t frame[]       = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};

  tx_send_data_return_value = SL_STATUS_FAIL;
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    multicast_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, received_status);

  tx_send_data_return_value = SL_STATUS_OK;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multicast_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));
}
