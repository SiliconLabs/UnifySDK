/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <string.h>

// Shared Unify includes
#include <unity.h>

// ZigPC includes
#include "zigpc_datastore_mock.h"
#include "zigpc_gateway_mock.h"
#include "zigpc_gateway_notify_mock.h"

#include "zigpc_net_mgmt_notify_int_mock.h"
#include "zigpc_net_mgmt_internal.h"

extern struct zigpc_net_mgmt_fsm fsm;

static const zigbee_eui64_t TEST_EUI64 = "\x20\xFB\x00\xA3\xBE\x88\x51\x33";
static const zigbee_install_code_t TEST_INSTALL_CODE
  = "\x20\xFB\x00\xA3\xBE\x88\x51\x33";
static const uint8_t TEST_INSTALL_CODE_LEN = 8;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  zigpc_net_mgmt_fsm_reset();
  zigpc_datastore_log_device_IgnoreAndReturn(SL_STATUS_OK);
}

void tearDown() {}

/**
 * @brief Check correct state transition from INIT to IDLE
 *
 */
void test_zigpc_net_mgmt_fsm_should_transition_to_idle_from_init_complete_event(
  void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t test_init_data;
  zigbee_ext_panid_t test_ext_pan_id = "\x12\x34\x56\x78\x9A\xBB\xF2\x48";

  zigpc_network_data_t nwk;
  memcpy(nwk.gateway_eui64, TEST_EUI64, sizeof(zigbee_eui64_t));
  memcpy(nwk.ext_panid, test_ext_pan_id, sizeof(zigbee_ext_panid_t));
  nwk.panid         = 0x3456;
  nwk.radio_channel = 0x12;

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_INIT;

  zigpc_gateway_on_network_init_t *net_init
    = &test_init_data.on_net_init_complete;

  memcpy(net_init->zigpc_eui64, nwk.gateway_eui64, sizeof(zigbee_eui64_t));
  net_init->zigpc_endpoint_id = 1;
  memcpy(net_init->zigpc_ext_panid, nwk.ext_panid, sizeof(zigbee_ext_panid_t));
  net_init->zigpc_panid         = nwk.panid;
  net_init->zigpc_radio_channel = nwk.radio_channel;

  zigpc_datastore_create_network_ExpectAndReturn(SL_STATUS_OK);
  zigpc_datastore_write_network_ExpectAndReturn(&nwk, SL_STATUS_OK);
  zigpc_datastore_log_network_IgnoreAndReturn(SL_STATUS_OK);

  zigpc_datastore_remove_device_ExpectAndReturn(net_init->zigpc_eui64,
                                                SL_STATUS_OK);
  zigpc_datastore_create_device_ExpectAndReturn(net_init->zigpc_eui64,
                                                SL_STATUS_OK);
  zigpc_datastore_write_device_ExpectAndReturn(net_init->zigpc_eui64,
                                               NULL,
                                               SL_STATUS_OK);
  zigpc_datastore_write_device_IgnoreArg_data();

  zigpc_datastore_create_endpoint_ExpectAndReturn(net_init->zigpc_eui64,
                                                  net_init->zigpc_endpoint_id,
                                                  SL_STATUS_OK);

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_network_init_ExpectWithArray(net_init->zigpc_eui64,
                                                     sizeof(zigbee_eui64_t));
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status
    = zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_INIT_COMPLETE,
                                    &test_init_data);

  // ASSERT (Handled by CMock)
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, test_status);
}

/**
 * @brief Check valid idle state transition
 *
 */
void test_zigpc_net_mgmt_fsm_should_handle_valid_state_transitions(void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t add_node_data;
  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
  memcpy(add_node_data.node_add_request.eui64,
         TEST_EUI64,
         sizeof(zigbee_eui64_t));
  memcpy(add_node_data.node_add_request.install_code,
         TEST_INSTALL_CODE,
         sizeof(zigbee_install_code_t));
  add_node_data.node_add_request.install_code_length = TEST_INSTALL_CODE_LEN;

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_gateway_add_node_ExpectWithArrayAndReturn(
    add_node_data.node_add_request.eui64,
    sizeof(zigbee_eui64_t),
    add_node_data.node_add_request.install_code,
    sizeof(zigbee_install_code_t),
    add_node_data.node_add_request.install_code_length,
    SL_STATUS_OK);

  // ACT
  sl_status_t test_status
    = zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_REQUEST,
                                    &add_node_data);

  // ASSERT (Handled by CMock)
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

/**
 * @brief Check invalid events received to a state
 *
 */
void test_zigpc_net_mgmt_fsm_should_handle_invalid_events_from_idle_state(void)
{
  TEST_ASSERT_EQUAL(
    SL_STATUS_INVALID_TYPE,
    zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_TIMEOUT, NULL));
  TEST_ASSERT_EQUAL(
    SL_STATUS_INVALID_PARAMETER,
    zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_MIN_VAL, NULL));
  TEST_ASSERT_EQUAL(
    SL_STATUS_INVALID_PARAMETER,
    zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_MAX_VAL, NULL));
}

/**
 * @brief Check unknown events received to a state
 *
 */
void test_zigpc_net_mgmt_fsm_should_handle_unknown_events_from_idle_state(void)
{
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER,
                    zigpc_net_mgmt_fsm_post_event(0xFF, NULL));
}

/**
 * @brief Check valid idle state transition
 *
 */
void test_zigpc_net_mgmt_fsm_node_add_request_should_call_gateway_api(void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t add_node_data;
  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_gateway_add_node_IgnoreAndReturn(SL_STATUS_OK);

  // ACT
  sl_status_t test_status
    = zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_REQUEST,
                                    &add_node_data);

  // ASSERT (Handled by CMock)
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
  TEST_ASSERT_EQUAL(ZIGPC_NET_MGMT_FSM_STATE_IDLE, fsm.state);
}

/**
 * @brief Check valid idle state transition
 *
 */
void test_zigpc_net_mgmt_fsm_node_add_start_event_should_transition_state(void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t add_node_data;
  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status
    = zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_START,
                                    &add_node_data);

  // ASSERT (Handled by CMock)
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
  TEST_ASSERT_EQUAL(ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD, fsm.state);
}

/**
 * @brief Check valid events received to add node state
 *
 */
void test_zigpc_net_mgmt_fsm_should_handle_complete_event_from_add_node_state(
  void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t node_added_data
    = {.on_node_add_complete = {
         .eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
       }};

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD;
  // Ensure that sending eui64 matches with the eui64 stored in FSM
  memcpy(fsm.joining_eui64,
         node_added_data.on_node_add_complete.eui64,
         sizeof(zigbee_eui64_t));

  {  // datastore expects
    zigpc_datastore_remove_device_ExpectAndReturn(fsm.joining_eui64,
                                                  SL_STATUS_OK);
    zigpc_datastore_create_device_ExpectAndReturn(fsm.joining_eui64,
                                                  SL_STATUS_OK);
    zigpc_datastore_write_device_ExpectAndReturn(fsm.joining_eui64,
                                                 NULL,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_device_IgnoreArg_data();
  }

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_node_added_Expect(
    node_added_data.on_node_add_complete.eui64);
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status
    = zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_COMPLETE,
                                    &node_added_data);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, test_status);
}

void test_zigpc_net_mgmt_fsm_should_handle_timeout_event_from_add_node_state(
  void)
{
  // ARRANGE
  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD;

  zigpc_gateway_network_permit_joins_ExpectAndReturn(false, SL_STATUS_OK);

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status
    = zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_TIMEOUT, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

/**
 * @brief Check valid events received to node interview state
 *
 */
void test_zigpc_net_mgmt_fsm_should_handle_interview_status_from_idle_state(
  void)
{
  zigpc_net_mgmt_fsm_data_t fsm_data
    = {.on_node_interview_status = {
         .eui64            = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
         .discovery_status = DISCOVERY_START,
       }};

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;

  // ARRANGE
  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEW_STATUS,
    &fsm_data);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, test_status);
}

/**
 * @brief Check device discovery success received in node interview state
 *
 */
void test_zigpc_net_mgmt_fsm_should_track_discovery_success_in_node_interview_state(
  void)
{
  zigpc_net_mgmt_fsm_data_t fsm_data
    = {.on_node_interview_status = {
         .eui64            = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
         .discovery_status = DISCOVERY_SUCCESS,
       }};

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW;

  // ARRANGE
  zigpc_net_mgmt_notify_node_interview_status_Expect(
    fsm_data.on_node_interview_status.eui64,
    true);

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEW_STATUS,
    &fsm_data);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

/**
 * @brief Check device discovery fail received in node interview state
 *
 */
void test_zigpc_net_mgmt_fsm_should_track_device_discovery_fail_in_node_interview_state(
  void)
{
  zigpc_net_mgmt_fsm_data_t fsm_data
    = {.on_node_interview_status = {
         .eui64            = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
         .discovery_status = DEVICE_DISCOVERY_FAIL,
       }};

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW;

  // ARRANGE
  zigpc_net_mgmt_notify_node_interview_status_Expect(
    fsm_data.on_node_interview_status.eui64,
    false);

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEW_STATUS,
    &fsm_data);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

/**
 * @brief Check endpoint discovery fail received in node interview state
 *
 */
void test_zigpc_net_mgmt_fsm_should_track_endpoint_discovery_fail_in_node_interview_state(
  void)
{
  zigpc_net_mgmt_fsm_data_t fsm_data
    = {.on_node_interview_status = {
         .eui64            = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
         .discovery_status = ENDPOINT_DISCOVERY_FAIL,
       }};

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW;

  // ARRANGE
  zigpc_net_mgmt_notify_node_interview_status_Expect(
    fsm_data.on_node_interview_status.eui64,
    false);

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEW_STATUS,
    &fsm_data);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

/**
 * @brief Check success path of handling node remove request from idle.
 *
 */
void test_zigpc_net_mgmt_fsm_should_handle_node_remove_request_from_idle_state(
  void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t remove_data;

  memcpy(remove_data.node_remove_request.eui64,
         TEST_EUI64,
         sizeof(zigbee_eui64_t));
  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;

  {  // datastore expects
    zigpc_datastore_read_device_ExpectAndReturn(TEST_EUI64, NULL, SL_STATUS_OK);
    zigpc_datastore_read_device_IgnoreArg_data();

    zigpc_datastore_write_device_ExpectAndReturn(TEST_EUI64,
                                                 NULL,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_device_IgnoreArg_data();
  }

  zigpc_gateway_remove_node_ExpectWithArrayAndReturn(TEST_EUI64,
                                                     sizeof(zigbee_eui64_t),
                                                     SL_STATUS_OK);

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_NODE_REMOVE_REQUEST,
    &remove_data);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

/**
 * @brief Check success path of handling node remove complete request.
 *
 */
void test_zigpc_net_mgmt_fsm_should_handle_complete_event_from_remove_node_state(
  void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t node_removed_data = {
    .on_node_remove_complete = {
      .eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
    },
  };

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE;
  // Ensure that sending eui64 matches with the eui64 stored in FSM
  memcpy(fsm.eui64_to_remove,
         node_removed_data.on_node_remove_complete.eui64,
         sizeof(zigbee_eui64_t));

  zigpc_datastore_remove_device_ExpectAndReturn(fsm.eui64_to_remove,
                                                SL_STATUS_OK);

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_node_removed_Expect(fsm.eui64_to_remove);
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_NODE_REMOVE_COMPLETE,
    &node_removed_data);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

void test_zigpc_net_mgmt_fsm_should_handle_timeout_event_from_node_remove_state(
  void)
{
  // ARRANGE
  zigbee_eui64_t test_eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};
  fsm.state                 = ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE;
  memcpy(fsm.eui64_to_remove, test_eui64, sizeof(zigbee_eui64_t));

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status
    = zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_TIMEOUT, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

void test_zigpc_net_mgmt_fsm_should_handle_state_change_request_event_from_idle_state(
  void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t change_request;

  change_request.state_change_request.requested_state
    = ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE;

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_add_requested_parameter_ExpectAndReturn("Unid",
                                                                SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_STATE_CHANGE_REQUEST,
    &change_request);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

void test_zigpc_net_mgmt_fsm_should_handle_filled_state_change_request_event_from_node_remove_state(
  void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t change_request;

  change_request.state_change_request.requested_state
    = ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE;
  change_request.state_change_request.param_eui64_filled = true;

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE;

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_gateway_remove_node_ExpectWithArrayAndReturn(
    change_request.state_change_request.param_eui64,
    sizeof(zigbee_eui64_t),
    SL_STATUS_OK);

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_STATE_CHANGE_REQUEST,
    &change_request);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}
