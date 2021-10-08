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

// Shared UIC includes
#include <unity.h>

// ZigPC includes
#include <zigpc_datastore_mock.h>
#include <zigpc_gateway_mock.h>
#include <zigpc_gateway_notify_mock.h>

#include "zigpc_net_mgmt_notify_int_mock.h"
#include "zigpc_net_mgmt_fsm.h"

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
  memcpy(test_init_data.on_net_init_complete.zigpc_eui64,
         nwk.gateway_eui64,
         sizeof(zigbee_eui64_t));
  memcpy(test_init_data.on_net_init_complete.zigpc_ext_panid,
         nwk.ext_panid,
         sizeof(zigbee_ext_panid_t));
  test_init_data.on_net_init_complete.zigpc_panid         = nwk.panid;
  test_init_data.on_net_init_complete.zigpc_radio_channel = nwk.radio_channel;

  zigpc_datastore_create_network_ExpectAndReturn(SL_STATUS_OK);
  zigpc_datastore_write_network_ExpectAndReturn(&nwk, SL_STATUS_OK);
  zigpc_datastore_log_network_IgnoreAndReturn(SL_STATUS_OK);

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_network_init_ExpectWithArray(
    test_init_data.on_net_init_complete.zigpc_eui64,
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
  zigpc_gateway_add_node_install_code_ExpectWithArrayAndReturn(
    add_node_data.node_add_request.eui64,
    sizeof(zigbee_eui64_t),
    add_node_data.node_add_request.install_code,
    sizeof(zigbee_install_code_t),
    add_node_data.node_add_request.install_code_length,
    SL_STATUS_OK);
  zigpc_gateway_network_permit_joins_ExpectAndReturn(true, SL_STATUS_OK);

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
  zigpc_gateway_add_node_install_code_IgnoreAndReturn(SL_STATUS_OK);
  zigpc_gateway_network_permit_joins_ExpectAndReturn(true, SL_STATUS_OK);

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
    zigpc_datastore_log_device_IgnoreAndReturn(SL_STATUS_OK);
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
void test_zigpc_net_mgmt_fsm_should_handle_interview_request_from_idle_state(
  void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t interview_data;

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
  memcpy(interview_data.node_interview_request.eui64,
         TEST_EUI64,
         sizeof(zigbee_eui64_t));

  {  // datastore expects
    zigpc_datastore_remove_device_children_ExpectAndReturn(TEST_EUI64,
                                                           SL_STATUS_OK);

    zigpc_datastore_read_device_ExpectAndReturn(TEST_EUI64, NULL, SL_STATUS_OK);
    zigpc_datastore_read_device_IgnoreArg_data();

    zigpc_datastore_write_device_ExpectAndReturn(TEST_EUI64,
                                                 NULL,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_device_IgnoreArg_data();
  }

  zigpc_gateway_interview_node_ExpectWithArrayAndReturn(
    interview_data.node_interview_request.eui64,
    sizeof(zigbee_eui64_t),
    SL_STATUS_OK);

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEW_REQUEST,
    &interview_data);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, test_status);
}

/**
 * @brief Check device discovered event received in node interview state
 *
 */
void test_zigpc_net_mgmt_fsm_should_track_device_discovery_event_in_node_interview_state(
  void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t node_discovered_data
    = {.on_node_discovered = {
         .eui64          = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
         .endpoint_count = 5,
       }};
  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW;
  memcpy(fsm.interview.eui64,
         node_discovered_data.on_node_discovered.eui64,
         sizeof(zigbee_eui64_t));

  {  // datastore expects
    zigpc_datastore_read_device_ExpectAndReturn(fsm.interview.eui64,
                                                NULL,
                                                SL_STATUS_OK);
    zigpc_datastore_read_device_IgnoreArg_data();

    zigpc_datastore_write_device_ExpectAndReturn(fsm.interview.eui64,
                                                 NULL,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_device_IgnoreArg_data();

    zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  }

  // ACT
  sl_status_t test_status
    = zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEWED,
                                    &node_discovered_data);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
  TEST_ASSERT_EQUAL_MEMORY(node_discovered_data.on_node_discovered.eui64,
                           fsm.interview.eui64,
                           sizeof(zigbee_eui64_t));
  TEST_ASSERT_EQUAL(node_discovered_data.on_node_discovered.endpoint_count,
                    fsm.interview.endpoint_total_count);
}

/**
 * @brief Check device endpoint discovered is received in node interview state
 *
 */
void test_zigpc_net_mgmt_fsm_should_track_single_device_endpoint_discovery_event_in_node_interview_state(
  void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t test_event_data
    = {.on_node_endpoint_discovered
       = {.eui64    = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
          .endpoint = {
            .endpoint_id   = 1,
            .cluster_count = 0,
            .client_cluster_count = 0,
          },
        },
      };

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW;
  memcpy(fsm.interview.eui64,
         test_event_data.on_node_endpoint_discovered.eui64,
         sizeof(zigbee_eui64_t));
  fsm.interview.endpoint_total_count = 1;

  {  // datastore expects
    zigpc_datastore_read_device_ExpectAndReturn(fsm.interview.eui64,
                                                NULL,
                                                SL_STATUS_OK);
    zigpc_datastore_read_device_IgnoreArg_data();

    zigbee_endpoint_t *ep
      = &test_event_data.on_node_endpoint_discovered.endpoint;
    zigpc_datastore_create_endpoint_ExpectAndReturn(fsm.interview.eui64,
                                                    ep->endpoint_id,
                                                    SL_STATUS_OK);

    zigpc_datastore_write_device_ExpectAndReturn(fsm.interview.eui64,
                                                 NULL,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_device_IgnoreArg_data();
  }

  zigpc_net_mgmt_notify_node_interview_status_Expect(
    test_event_data.on_node_endpoint_discovered.eui64,
    true);
  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_NODE_ENDPOINT_INTERVIEWED,
    &test_event_data);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
  TEST_ASSERT_EQUAL(1, fsm.interview.endpoint_discovered_count);
}

/**
 * @brief Check device endpoint discovered is received in node interview state
 *
 */
void test_zigpc_net_mgmt_fsm_should_track_multiple_device_endpoint_discovery_event_in_node_interview_state(
  void)
{
  // ARRANGE
  zigpc_net_mgmt_fsm_data_t test_event_data[2] = {
    {
      .on_node_endpoint_discovered = {
        .eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
        .endpoint = {
          .endpoint_id = 1,
          .cluster_count = 2,
          .cluster_list = {
            {.cluster_id = 5},
            {.cluster_id = 6},
          },
          .client_cluster_count = 1,
          .client_cluster_list = {
            {.cluster_id = 10},
            {.cluster_id = 12},
          },
        }
      }
    },
    {
      .on_node_endpoint_discovered = {
        .eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
        .endpoint = {
          .endpoint_id = 3,
          .cluster_count = 0,
        }
      }
    }
  };

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW;
  memcpy(fsm.interview.eui64,
         test_event_data[0].on_node_endpoint_discovered.eui64,
         sizeof(zigbee_eui64_t));
  fsm.interview.endpoint_total_count      = 2;
  fsm.interview.endpoint_discovered_count = 0;

  {  // datastore expects
    zigpc_datastore_read_device_ExpectAndReturn(fsm.interview.eui64,
                                                NULL,
                                                SL_STATUS_OK);
    zigpc_datastore_read_device_IgnoreArg_data();

    zigbee_endpoint_t *ep
      = &test_event_data[0].on_node_endpoint_discovered.endpoint;
    zigpc_datastore_create_endpoint_ExpectAndReturn(fsm.interview.eui64,
                                                    ep->endpoint_id,
                                                    SL_STATUS_OK);

    for (size_t i = 0; i < ep->cluster_count; i++) {
      zigpc_datastore_create_cluster_ExpectAndReturn(
        fsm.interview.eui64,
        ep->endpoint_id,
        ZCL_CLUSTER_SERVER_SIDE,
        ep->cluster_list[i].cluster_id,
        SL_STATUS_OK);
    }
    for (size_t i = 0; i < ep->client_cluster_count; i++) {
      zigpc_datastore_create_cluster_ExpectAndReturn(
        fsm.interview.eui64,
        ep->endpoint_id,
        ZCL_CLUSTER_CLIENT_SIDE,
        ep->client_cluster_list[i].cluster_id,
        SL_STATUS_OK);
    }

    zigpc_datastore_write_device_ExpectAndReturn(fsm.interview.eui64,
                                                 NULL,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_device_IgnoreArg_data();
  }

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_NODE_ENDPOINT_INTERVIEWED,
    &test_event_data[0]);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
  TEST_ASSERT_EQUAL(1, fsm.interview.endpoint_discovered_count);

  // ARRANGE

  {  // datastore expects
    zigpc_datastore_read_device_ExpectAndReturn(fsm.interview.eui64,
                                                NULL,
                                                SL_STATUS_OK);
    zigpc_datastore_read_device_IgnoreArg_data();

    zigbee_endpoint_t *ep
      = &test_event_data[1].on_node_endpoint_discovered.endpoint;
    zigpc_datastore_create_endpoint_ExpectAndReturn(fsm.interview.eui64,
                                                    ep->endpoint_id,
                                                    SL_STATUS_OK);

    zigpc_datastore_write_device_ExpectAndReturn(fsm.interview.eui64,
                                                 NULL,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_device_IgnoreArg_data();
  }

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  /* Only expect the state to change once after all endpoint interview data
   * is received
   */
  zigpc_net_mgmt_notify_node_interview_status_Expect(
    test_event_data[0].on_node_endpoint_discovered.eui64,
    true);

  // ACT
  test_status = zigpc_net_mgmt_fsm_post_event(
    ZIGPC_NET_MGMT_FSM_EVENT_NODE_ENDPOINT_INTERVIEWED,
    &test_event_data[1]);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
  TEST_ASSERT_EQUAL(2, fsm.interview.endpoint_discovered_count);
}

void test_zigpc_net_mgmt_fsm_should_handle_timeout_event_from_node_interview_state(
  void)
{
  // ARRANGE
  zigbee_eui64_t test_eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};
  fsm.state                 = ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW;
  memcpy(fsm.interview.eui64, test_eui64, sizeof(zigbee_eui64_t));

  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW;

  {  // datastore expects
    zigpc_datastore_read_device_ExpectAndReturn(fsm.interview.eui64,
                                                NULL,
                                                SL_STATUS_OK);
    zigpc_datastore_read_device_IgnoreArg_data();

    zigpc_datastore_write_device_ExpectAndReturn(fsm.interview.eui64,
                                                 NULL,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_device_IgnoreArg_data();
  }

  zigpc_net_mgmt_notify_clear_requested_parameter_list_Expect();
  zigpc_net_mgmt_notify_state_update_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_notify_state_update_IgnoreArg_next_supported_states();
  zigpc_net_mgmt_notify_node_interview_status_Expect(test_eui64, false);

  // ACT
  sl_status_t test_status
    = zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_TIMEOUT, NULL);

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
