/**
 * @file zwave_network_management_test.c
 * @brief Test of zwave_network_management
 *
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 */
#include <string.h>

#include "zwave_network_management.h"
#include "zwave_network_management_fixt.h"

#include "contiki_test_helper.h"
#include "zwave_s2_network_mock.h"
#include "zwave_s2_keystore_mock.h"
#include "zwave_s0_network_mock.h"
#include "zwave_tx_mock.h"
#include "zwapi_init_mock.h"
#include "zwapi_protocol_controller_mock.h"
#include "zwapi_protocol_mem_mock.h"
#include "zwapi_protocol_basis_mock.h"

#include "zwave_controller_mock.h"
#include "zwave_controller_internal_mock.h"
#include "zwave_controller_utils_mock.h"
#include "zwave_controller_callbacks_mock.h"
#include "zwave_controller_storage_mock.h"
#include "zwave_smartstart_management_mock.h"

#include "ZW_classcmd.h"
#include "sl_log.h"
#include "zwave_s2_network.h"
#include "process.h"

#define KEY_CLASS_S0                 0x80
#define KEY_CLASS_S2_UNAUTHENTICATED 0x01
#define KEY_CLASS_S2_AUTHENTICATED   0x02
#define KEY_CLASS_S2_ACCESS          0x04
#define NODE_INFO_MAX_SIZE           64

void my_sec2_inclusion_stub(const zwave_s2_network_callbacks_t *cb,
                            int cmock_num_calls);
void my_sec0_inclusion_stub(const s0_on_bootstrapping_complete_cb cb,
                            int cmock_num_calls);

zwave_dsk_t dsk;
zwave_keyset_t granted_keys = KEY_CLASS_S0 | KEY_CLASS_S2_UNAUTHENTICATED
                              | KEY_CLASS_S2_AUTHENTICATED
                              | KEY_CLASS_S2_ACCESS;
static zwave_node_id_t node_id = 42;
zwave_node_info_t nif_buffer;
zwave_node_info_t *sample_nif       = &nif_buffer;
zwave_protocol_t inclusion_protocol = PROTOCOL_ZWAVE;

static uint8_t node_add_nif_with_s2[] = {
  BASIC_TYPE_END_NODE,
  GENERIC_TYPE_SENSOR_NOTIFICATION,
  SPECIFIC_TYPE_NOTIFICATION_SENSOR,
  COMMAND_CLASS_ZWAVEPLUS_INFO,
  COMMAND_CLASS_SECURITY_2,
};

static uint8_t node_add_nif_no_s2[] = {
  BASIC_TYPE_END_NODE,
  GENERIC_TYPE_SENSOR_NOTIFICATION,
  SPECIFIC_TYPE_NOTIFICATION_SENSOR,
  COMMAND_CLASS_ZWAVEPLUS_INFO,
  COMMAND_CLASS_TRANSPORT_SERVICE,
};

//Just a stub
//void zwave_sl_log_dsk(void *d) {}

static zwave_s2_network_callbacks_t my_sec2_inclusion_cb;
static s0_on_bootstrapping_complete_cb my_sec0_inclusion_cb;

void my_sec2_inclusion_stub(const zwave_s2_network_callbacks_t *cb,
                            int cmock_num_calls)
{
  my_sec2_inclusion_cb = *cb;
}
void my_sec0_inclusion_stub(const s0_on_bootstrapping_complete_cb cb,
                            int cmock_num_calls)
{
  my_sec0_inclusion_cb = cb;
}

// Assign_return_route callback
static cmock_zwapi_protocol_controller_func_ptr2
  zwapi_assign_return_callback_save;
sl_status_t zwapi_assign_return_route_stub(
  zwave_node_id_t src_node_id,
  zwave_node_id_t dest_node_id,
  cmock_zwapi_protocol_controller_func_ptr2 completedFunc,
  int cmock_retval)
{
  zwapi_assign_return_callback_save = completedFunc;
  return SL_STATUS_OK;
}

PROCESS_NAME(zwave_network_management_process);

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  zwapi_assign_return_callback_save = NULL;
}

void setUp()
{
  zwave_controller_register_reset_step_ExpectAndReturn(NULL, 5, SL_STATUS_OK);
  zwave_controller_register_reset_step_IgnoreArg_step_function();
  zwave_controller_storage_as_set_node_s2_capable_IgnoreAndReturn(SL_STATUS_OK);

  zwave_controller_is_reset_ongoing_IgnoreAndReturn(false);

  zwave_s2_set_network_callbacks_Stub(my_sec2_inclusion_stub);
  zwave_s0_set_network_callbacks_Stub(my_sec0_inclusion_stub);
  zwave_s2_keystore_get_dsk_Ignore();
  zwave_sl_log_dsk_Ignore();

  zwapi_memory_get_ids_ExpectAndReturn(0, 0, SL_STATUS_OK);
  zwapi_memory_get_ids_IgnoreArg_home_id();
  zwapi_memory_get_ids_IgnoreArg_node_id();

  // Initialize the fixture
  zwave_nodemask_t node_list;
  memset(node_list, 0, sizeof(zwave_nodemask_t));
  node_list[0]  = 0b00101101;  // 4 nodes in the network, NodeID 1, 3, 4 and 6
  node_list[31] = 1;  // 5 nodes in the network, NodeID 1, 3, 4 and 6, 256
  zwapi_get_full_node_list_IgnoreAndReturn(SL_STATUS_OK);
  zwapi_get_full_node_list_ReturnMemThruPtr_node_list(node_list,
                                                      sizeof(zwave_nodemask_t));

  zwapi_add_node_to_network_IgnoreAndReturn(SL_STATUS_OK);
  zwapi_remove_node_from_network_IgnoreAndReturn(SL_STATUS_OK);
  zwapi_set_learn_mode_IgnoreAndReturn(SL_STATUS_OK);
  zwapi_get_controller_capabilities_IgnoreAndReturn(
    (CONTROLLER_NODEID_SERVER_PRESENT | CONTROLLER_IS_SUC));

  zwave_network_management_fixt_setup();

  // De-activate SmartStart learn mode for all tests by default
  zwave_network_management_enable_smart_start_learn_mode(false);

  contiki_test_helper_init();

  process_start(&zwave_network_management_process, NULL);
  contiki_test_helper_run(0);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

static on_zwave_tx_send_data_complete_t my_zwave_tx_send_data_cb;
sl_status_t my_zwave_tx_send_data_stub(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session,
  int cmock_num_calls)
{
  my_zwave_tx_send_data_cb = on_send_complete;
  return SL_STATUS_OK;
}

/* The purpose of this is to store the callback ptr sent to
 * zwapi_remove_failed_node
 */
static cmock_zwapi_protocol_controller_func_ptr5
  remove_failed_node_callback_save;
static uint8_t my_remove_failed_node_stub(
  zwave_node_id_t NodeID,
  cmock_zwapi_protocol_controller_func_ptr5 completedFunc,
  int cmock_num_calls)
{
  remove_failed_node_callback_save = completedFunc;
  return SL_STATUS_OK;
}

/* The purpose of this is to store the callback ptr sent to
 * zwapi_add_node_to_network
 */
static cmock_zwapi_protocol_controller_func_ptr18 add_node_callback_save;
static sl_status_t
  my_add_node_stub(uint8_t bMode,
                   const uint8_t *dsk,
                   cmock_zwapi_protocol_controller_func_ptr18 completedFunc,
                   int cmock_num_calls)
{
  add_node_callback_save = completedFunc;
  return SL_STATUS_OK;
}

static cmock_zwapi_protocol_controller_func_ptr4 my_set_default_callbacks_save;
static sl_status_t

  my_set_default_stub(cmock_zwapi_protocol_controller_func_ptr4 func, int n)
{
  my_set_default_callbacks_save = func;
  return SL_STATUS_OK;
}

/* To store the callback ptr to sent to zwapi_remove_failed_node*/
static cmock_zwapi_protocol_controller_func_ptr5
  failed_node_remove_callback_save;

static uint8_t
  failed_node_remove_stub(zwave_node_id_t NodeID,
                          cmock_zwapi_protocol_controller_func_ptr5 funptr,
                          int cmock_num_calls)
{
  failed_node_remove_callback_save = funptr;
  return SL_STATUS_OK;
}

/* To store the callback ptr to sent to zwapi_remove_node_from_network*/
static cmock_zwapi_protocol_controller_func_ptr18 remove_node_callback_save;
static sl_status_t
  remove_node_stub(uint8_t bMode,
                   cmock_zwapi_protocol_controller_func_ptr18 funptr,
                   int cmoke_num_calls)
{
  remove_node_callback_save = funptr;
  return SL_STATUS_OK;
}

static cmock_zwapi_protocol_controller_func_ptr18 my_s2_add_node_callback_save;
static sl_status_t
  my_s2_add_node_stub(uint8_t bMode,
                      cmock_zwapi_protocol_controller_func_ptr18 funptr,
                      int cmoke_num_calls)
{
  my_s2_add_node_callback_save = funptr;
  return SL_STATUS_OK;
}
static cmock_zwapi_protocol_controller_func_ptr1
  request_neighbor_update_callback_save;
static zwave_node_id_t request_neighbor_node_id = 0x00;
sl_status_t my_zwapi_request_neighbor_update_stub(
  zwave_node_id_t bNodeID,
  cmock_zwapi_protocol_controller_func_ptr1 funptr,
  int cmoke_num_calls)
{
  request_neighbor_node_id              = bNodeID;
  request_neighbor_update_callback_save = funptr;
  return SL_STATUS_OK;
}

// Storing the callback ptr to zwapi_set_learn_mode
static cmock_zwapi_protocol_controller_func_ptr13
  my_zwapi_set_learn_mode_cb_save;
static sl_status_t my_zwapi_set_learn_mode_stub(
  uint8_t learn_mode,
  cmock_zwapi_protocol_controller_func_ptr13 completedFunc,
  int cmock_num_calls)
{
  my_zwapi_set_learn_mode_cb_save = completedFunc;
  return SL_STATUS_OK;
}

void expect_smart_start_enable()
{
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_SMART_START
                                              | ADD_NODE_OPTION_NETWORK_WIDE,
                                            0,
                                            SL_STATUS_OK);
}

void exepct_zwapi_add_smartstart_node_to_network()
{
  /* Tell cmock to expect a call to zwapi_add_smartstart_node_to_network()
   * with arguments, (ADD_NODE_HOME_ID, dsk) and when it happens
   * cmock should return SL_STATUS_OK */
  uint8_t mode = ADD_NODE_HOME_ID;
  if (inclusion_protocol == PROTOCOL_ZWAVE_LONG_RANGE) {
    mode |= ADD_NODE_OPTION_LONG_RANGE;
  }

  zwapi_add_smartstart_node_to_network_ExpectAndReturn(mode,
                                                       &dsk[8],
                                                       0,
                                                       SL_STATUS_OK);
  /* Tell cmock to ignore one of the arguments (completedFunc)
   * to zwapi_add_smartstart_node_to_network()
   */
  zwapi_add_smartstart_node_to_network_IgnoreArg_completedFunc();
}

/* This test will test the timeout in waiting for ADD_NODE_STATUS_NODE_FOUND
 * from protocol. Timeout: 60 seconds
 * See Application Programmers guide:  ZW_AddNodeToNetwork() section */
void test_zwave_network_management_add_node_smartstart_node_found_timeout()
{
  //We expect sm mode to be enabled
  expect_smart_start_enable();
  zwave_network_management_enable_smart_start_add_mode(true);
  /* Check if the Network management state is idle */
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  zwave_controller_on_state_updated_Ignore();

  exepct_zwapi_add_smartstart_node_to_network();
  /* Call Network management */
  zwave_network_management_add_node_with_dsk(dsk, granted_keys, PROTOCOL_ZWAVE);

  // Fire the event
  contiki_test_helper_run(0);

  /* When we return here the state of NM should be NM_WAITING_FOR_ADD */
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  // Tell cmock to expect (ADD_NODE_STOP)
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, 0);

  // Tell cmock to expect SmartStart enable
  expect_smart_start_enable();

  //Tell cmock to expect error
  zwave_controller_on_error_Expect(
    ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL);

  //Fire the timeout event
  contiki_test_helper_run(ADD_REMOVE_TIMEOUT * 10);

  //NM should be back in NM_IDLE state
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/* This tests time out waiting for theADD_NODE_STATUS_ADDING_END_NODE status
 * if it does not receive the indication within a time period after receiving
 * the ADD_NODE_STATUS_NODE_FOUND status. timeout: ? seconds
 * See Application Programmers guide:  ZW_AddNodeToNetwork() section */
void test_zwave_network_management_add_node_smartstart_end_node_found_timeout()
{
  //We expect sm mode to be enabled
  expect_smart_start_enable();

  zwave_network_management_enable_smart_start_add_mode(true);
  /* Check if the Network management state is idle */
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  exepct_zwapi_add_smartstart_node_to_network();
  zwave_controller_on_state_updated_Ignore();
  /* Instead add our own callback for cmock nd it will basiclly save the
   * callback(completedFunc) funciton sent to
   * zwapi_add_smartstart_node_to_network(). This callback function will be
   * saved at on_node_add_started_Expect() which is used in later steps*/
  zwapi_add_smartstart_node_to_network_AddCallback(my_add_node_stub);

  /* Call Network management */
  zwave_network_management_add_node_with_dsk(dsk, granted_keys, PROTOCOL_ZWAVE);

  // Fire the event
  contiki_test_helper_run(0);
  /* When we return here the state of NM should be NM_WAITING_FOR_ADD */
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  /*MOCK the serial api learn mode info and pass it to the callback */
  LEARN_INFO node_found = {ADD_NODE_STATUS_NODE_FOUND, node_id};
  add_node_callback_save(&node_found);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP_FAILED, NULL, 0);
  expect_smart_start_enable();

  //Tell cmock to expect error
  zwave_controller_on_error_Expect(
    ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL);

  //Fire the timeout event
  contiki_test_helper_run(CLOCK_SECOND * 60);

  //NM should be back in NM_IDLE state
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void setup_mock_for_zwave_tx_send_data(sl_status_t ret)
{
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_AddCallback(NULL);

  if (ret == SL_STATUS_OK) {
    zwave_tx_send_data_AddCallback(my_zwave_tx_send_data_stub);
  }
}
void setup_zwapi_remove_failed_node_mock(int set_cb)
{
  zwapi_remove_failed_node_ExpectAndReturn(node_id,
                                           0,
                                           ZW_FAILED_NODE_REMOVE_STARTED);
  zwapi_remove_failed_node_IgnoreArg_completedFunc();
  zwapi_remove_failed_node_AddCallback(NULL);
  if (set_cb) {
    zwapi_remove_failed_node_AddCallback(my_remove_failed_node_stub);
  }
}

/* This tests getting NM_EV_ADD_FAILED in NM_WAIT_FOR_PROTOCOL
 * while waiting for the ADD_NODE_STATUS_PROTOCOL_DONE status
 *  after receiving
 * the ADD_NODE_STATUS_NODE_FOUND status.*/
void test_zwave_network_management_add_node_smartstart_prot_done_timeout()
{
  //We expect sm mode to be enabled
  expect_smart_start_enable();

  zwave_network_management_enable_smart_start_add_mode(true);
  /* Check if the Network management state is idle */
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  exepct_zwapi_add_smartstart_node_to_network();
  /* Instead add our own callback for cmock nd it will basiclly save the
   * callback(completedFunc) funciton sent to
   * zwapi_add_smartstart_node_to_network(). This callback function will be
   * saved at on_node_add_started_Expect() which is used in later steps*/
  zwapi_add_smartstart_node_to_network_AddCallback(my_add_node_stub);
  zwave_controller_on_node_id_assigned_Expect(node_id, PROTOCOL_ZWAVE);
  zwave_controller_on_state_updated_Ignore();

  /* Call Network management */
  zwave_network_management_add_node_with_dsk(dsk, granted_keys, PROTOCOL_ZWAVE);

  // Fire the event
  contiki_test_helper_run(0);
  /* When we return here the state of NM should be NM_WAITING_FOR_ADD */
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  /*MOCK the serial api learn mode info and pass it to the callback */
  LEARN_INFO node_found = {ADD_NODE_STATUS_NODE_FOUND, node_id};
  add_node_callback_save(&node_found);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  contiki_test_helper_run(0);

  node_found.bStatus = ADD_NODE_STATUS_ADDING_END_NODE;
  node_found.bSource = node_id;
  node_found.bLen
    = sizeof(node_add_nif_no_s2);  //nif_buffer and sample_nif are same
  node_found.pCmd = node_add_nif_no_s2;

  zwave_command_class_list_unpack_Expect(NULL,
                                         &node_found.pCmd[3],
                                         node_found.bLen - 3);
  zwave_command_class_list_unpack_ReturnThruPtr_node_info(sample_nif);
  zwave_command_class_list_unpack_IgnoreArg_node_info();
  add_node_callback_save(&node_found);

  /* MOCK the serial api learn mode info again for next callback*/
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());

  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP_FAILED, NULL, 0);

  //Tell cmock to expect error
  zwave_controller_on_error_Expect(
    ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL);

  // Advance by the timeout value: ADD_NODE_PROTOCOL_NEIGHBOR_DISCOVERY_TIMEOUT
  contiki_test_helper_run(ADD_NODE_PROTOCOL_NEIGHBOR_DISCOVERY_TIMEOUT);

  //NM should now wait for self destruct
  TEST_ASSERT_EQUAL(NM_SEND_NOP, zwave_network_management_get_state());

  uint8_t nop_frame[1] = {0};
  zwave_tx_send_data_ExpectAndReturn(0,
                                     sizeof(nop_frame),
                                     nop_frame,
                                     0,
                                     0,
                                     0,
                                     0,
                                     SL_STATUS_OK);
  setup_mock_for_zwave_tx_send_data(SL_STATUS_OK);
  /* Setup zwapi_remove_failed_node() stuff. Also save the callback*/
  setup_zwapi_remove_failed_node_mock(1);

  /*Expect SmartStart enabled*/
  expect_smart_start_enable();

  /* Transition from NM_WAIT_FOR_SECURE_ADD to NM_WAIT_FOR_SECURE_ADD*/
  contiki_test_helper_run(SMART_START_SELF_DESTRUCT_TIMEOUT);

  my_zwave_tx_send_data_cb(TRANSMIT_COMPLETE_FAIL, 0, 0);
  contiki_test_helper_run(0);

  /* call the callback send to zwapi_remove_failed_node() with
   * ZW_FAILED_NODE_REMOVED*/
  remove_failed_node_callback_save(ZW_FAILED_NODE_REMOVED);

  // Check the zwave_controller_on_node_deleted() is called with the nodeID of the
  // node that is removed from the network.
  zwave_controller_on_node_deleted_Expect(node_id);

  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}
/* This tests time out waiting for the ADD_NODE_STATUS_PROTOCOL_DONE status
 * if it does not receive the indication within a time period after receiving
 * the ADD_NODE_STATUS_NODE_FOUND status.
 * The time period depends on the network size and the node types in the network.
 * See Application Programmers guide:  ZW_AddNodeToNetwork() section */

void test_zwave_network_management_add_node_smartstart_prot_done_add_failed()
{
  //We expect sm mode to be enabled
  expect_smart_start_enable();

  zwave_network_management_enable_smart_start_add_mode(true);
  /* Check if the Network management state is idle */
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  exepct_zwapi_add_smartstart_node_to_network();
  /* Instead add our own callback for cmock nd it will basiclly save the
   * callback(completedFunc) funciton sent to
   * zwapi_add_smartstart_node_to_network(). This callback function will be
   * saved at on_node_add_started_Expect() which is used in later steps*/
  zwapi_add_smartstart_node_to_network_AddCallback(my_add_node_stub);
  zwave_controller_on_node_id_assigned_Expect(node_id, PROTOCOL_ZWAVE);
  zwave_controller_on_state_updated_Ignore();

  /* Call Network management */
  zwave_network_management_add_node_with_dsk(dsk, granted_keys, PROTOCOL_ZWAVE);

  // Fire the event
  contiki_test_helper_run(0);
  /* When we return here the state of NM should be NM_WAITING_FOR_ADD */
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  /*MOCK the serial api learn mode info and pass it to the callback */
  LEARN_INFO node_found = {ADD_NODE_STATUS_NODE_FOUND, node_id};
  add_node_callback_save(&node_found);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  contiki_test_helper_run(0);

  node_found.bStatus = ADD_NODE_STATUS_ADDING_END_NODE;
  node_found.bSource = node_id;
  node_found.bLen
    = sizeof(node_add_nif_no_s2);  //nif_buffer and sample_nif are same
  node_found.pCmd = node_add_nif_no_s2;

  zwave_command_class_list_unpack_Expect(NULL,
                                         &node_found.pCmd[3],
                                         node_found.bLen - 3);
  zwave_command_class_list_unpack_ReturnThruPtr_node_info(sample_nif);
  zwave_command_class_list_unpack_IgnoreArg_node_info();
  add_node_callback_save(&node_found);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());

  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP_FAILED, NULL, 0);

  //Tell cmock to expect error
  zwave_controller_on_error_Expect(
    ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL);

  node_found.bStatus = ADD_NODE_STATUS_FAILED;
  add_node_callback_save(&node_found);
  contiki_test_helper_run(0);

  //NM should now wait for self destruct
  TEST_ASSERT_EQUAL(NM_SEND_NOP, zwave_network_management_get_state());
  uint8_t nop_frame[1] = {0};
  zwave_tx_send_data_ExpectAndReturn(0,
                                     sizeof(nop_frame),
                                     nop_frame,
                                     0,
                                     0,
                                     0,
                                     0,
                                     SL_STATUS_OK);
  setup_mock_for_zwave_tx_send_data(SL_STATUS_OK);
  /* Setup zwapi_remove_failed_node() stuff. Also save the callback*/
  setup_zwapi_remove_failed_node_mock(1);

  /*Expect SmartStart enabled*/
  expect_smart_start_enable();

  /* Transition from NM_WAIT_FOR_SECURE_ADD to NM_WAIT_FOR_SECURE_ADD*/
  contiki_test_helper_run(SMART_START_SELF_DESTRUCT_TIMEOUT);

  my_zwave_tx_send_data_cb(TRANSMIT_COMPLETE_FAIL, 0, 0);
  contiki_test_helper_run(0);

  /* call the callback send to zwapi_remove_failed_node() with
   * ZW_FAILED_NODE_REMOVED*/
  remove_failed_node_callback_save(ZW_FAILED_NODE_REMOVED);

  // Check the zwave_controller_on_node_deleted() is called with the nodeID of the
  // node that is removed from the network.
  zwave_controller_on_node_deleted_Expect(node_id);

  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/* Tests sending 0xff as status to add_node_status_update */
void test_add_node_status_update_unknown_status()
{
  //We expect sm mode to be enabled
  expect_smart_start_enable();

  zwave_network_management_enable_smart_start_add_mode(true);
  /* Check if the Network management state is idle */
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  exepct_zwapi_add_smartstart_node_to_network();
  /* Instead add our own callback for cmock nd it will basiclly save the
   * callback(completedFunc) funciton sent to
   * zwapi_add_smartstart_node_to_network(). This callback function will be
   * saved at on_node_add_started_Expect() which is used in later steps*/
  zwapi_add_smartstart_node_to_network_AddCallback(my_add_node_stub);
  zwave_controller_on_node_id_assigned_Expect(node_id, PROTOCOL_ZWAVE);
  zwave_controller_on_state_updated_Ignore();

  /* Call Network management */
  zwave_network_management_add_node_with_dsk(dsk, granted_keys, PROTOCOL_ZWAVE);

  // Fire the event
  contiki_test_helper_run(0);
  /* When we return here the state of NM should be NM_WAITING_FOR_ADD */
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  /*MOCK the serial api learn mode info and pass it to the callback */
  LEARN_INFO node_found = {ADD_NODE_STATUS_NODE_FOUND, node_id};
  add_node_callback_save(&node_found);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  contiki_test_helper_run(0);

  node_found.bStatus = ADD_NODE_STATUS_ADDING_END_NODE;
  node_found.bSource = node_id;
  node_found.bLen
    = sizeof(node_add_nif_no_s2);  //nif_buffer and sample_nif are same
  node_found.pCmd = node_add_nif_no_s2;

  zwave_command_class_list_unpack_Expect(NULL,
                                         &node_found.pCmd[3],
                                         node_found.bLen - 3);
  zwave_command_class_list_unpack_ReturnThruPtr_node_info(sample_nif);
  zwave_command_class_list_unpack_IgnoreArg_node_info();
  add_node_callback_save(&node_found);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());

  node_found.bStatus = 0xFF;
  add_node_callback_save(&node_found);
  contiki_test_helper_run(0);

  //NM should be still NM_WAIT_FOR_PROTOCOL
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());
}

void test_nif_length_invalid_max()
{
  int i;

  for (i = 0; i < sizeof(dsk); i++) {
    dsk[i] = i;
  }
  //We expect sm mode to be enabled
  expect_smart_start_enable();

  zwave_network_management_enable_smart_start_add_mode(true);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  /* Check if the Network management state is idle */
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  exepct_zwapi_add_smartstart_node_to_network();
  /* Instead add our own callback for cmock nd it will basiclly save the
   * callback(completedFunc) funciton sent to
   * zwapi_add_smartstart_node_to_network(). This callback function will be
   * saved at on_node_add_started_Expect() which is used in later steps*/
  zwapi_add_smartstart_node_to_network_AddCallback(my_add_node_stub);
  zwave_controller_on_state_updated_Ignore();

  /* Call Network management */
  zwave_network_management_add_node_with_dsk(dsk, granted_keys, PROTOCOL_ZWAVE);

  contiki_test_helper_run(0);
  /* When we return here the state of NM should be NM_WAITING_FOR_ADD */
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  /*MOCK the serial api learn mode info and pass it to the callback */
  LEARN_INFO node_found = {ADD_NODE_STATUS_NODE_FOUND, node_id};
  add_node_callback_save(&node_found);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  uint8_t node_add_nif[NODE_INFO_MAX_SIZE + 1] = {
    BASIC_TYPE_END_NODE,
    GENERIC_TYPE_SENSOR_NOTIFICATION,
    SPECIFIC_TYPE_NOTIFICATION_SENSOR,
    COMMAND_CLASS_ZWAVEPLUS_INFO,
    COMMAND_CLASS_TRANSPORT_SERVICE,
  };

  node_found.bStatus = ADD_NODE_STATUS_ADDING_END_NODE;
  node_found.bSource = node_id;
  node_found.bLen    = NODE_INFO_MAX_SIZE + 1;
  node_found.pCmd    = node_add_nif;

  add_node_callback_save(&node_found);
  zwave_controller_on_state_updated_Ignore();

  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP_FAILED, NULL, 0);
  zwapi_add_node_to_network_IgnoreArg_completedFunc();
  /*Expect error returned ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL*/
  zwave_controller_on_error_Expect(
    ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());
  /*Expect SmartStart enabled*/
  expect_smart_start_enable();

  /* Transition from NM_WAIT_FOR_SECURE_ADD to NM_WAIT_FOR_SECURE_ADD*/
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_abort_in_non_idle()
{
  TEST_ASSERT_EQUAL(SL_STATUS_IDLE, zwave_network_management_abort());
}

void idle_common()
{
  int i;

  for (i = 0; i < sizeof(dsk); i++) {
    dsk[i] = i;
  }
  //We expect sm mode to be enabled
  expect_smart_start_enable();

  zwave_network_management_enable_smart_start_add_mode(true);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  /* Check if the Network management state is idle */
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  exepct_zwapi_add_smartstart_node_to_network();
  /* Instead add our own callback for cmock nd it will basiclly save the
   * callback(completedFunc) funciton sent to
   * zwapi_add_smartstart_node_to_network(). This callback function will be
   * saved at on_node_add_started_Expect() which is used in later steps*/
  zwapi_add_smartstart_node_to_network_AddCallback(my_add_node_stub);
  zwave_controller_on_state_updated_Ignore();

  /* Call Network management */
  zwave_network_management_add_node_with_dsk(dsk,
                                             granted_keys,
                                             inclusion_protocol);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());
}

void test_zwave_network_management_add_node_with_dsk_when_not_idle()
{
  idle_common();
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_network_management_add_node_with_dsk(dsk,
                                                               granted_keys,
                                                               PROTOCOL_ZWAVE));
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());
}

void test_zwave_network_management_set_default_when_not_idle()
{
  idle_common();
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_network_management_add_node_with_dsk(dsk,
                                                               granted_keys,
                                                               PROTOCOL_ZWAVE));
  TEST_ASSERT_EQUAL(SL_STATUS_BUSY, zwave_network_management_set_default());
}

void test_zwave_network_management_remove_node_when_not_idle()
{
  idle_common();
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_network_management_add_node_with_dsk(dsk,
                                                               granted_keys,
                                                               PROTOCOL_ZWAVE));
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwave_network_management_remove_node());
}

/* Common SmartStart code until dsk accepting. This code is used in
 * most of the unit tests */
void smart_start_common()
{
  sample_nif->listening_protocol
    = ZWAVE_NODE_INFO_LISTENING_PROTOCOL_ROUTING_MASK;
  sample_nif->optional_protocol
    = ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_END_NODE_MASK;
  sample_nif->basic_device_class        = BASIC_TYPE_END_NODE;
  sample_nif->generic_device_class      = GENERIC_TYPE_SENSOR_NOTIFICATION;
  sample_nif->specific_device_class     = SPECIFIC_TYPE_NOTIFICATION_SENSOR;
  sample_nif->command_class_list_length = 2;
  sample_nif->command_class_list[0]     = COMMAND_CLASS_ZWAVEPLUS_INFO;
  sample_nif->command_class_list[1]     = COMMAND_CLASS_TRANSPORT_SERVICE;

  int i;

  for (i = 0; i < sizeof(dsk); i++) {
    dsk[i] = i;
  }
  //We expect sm mode to be enabled
  expect_smart_start_enable();
  zwave_network_management_enable_smart_start_add_mode(true);

  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  /* Check if the Network management state is idle */
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  exepct_zwapi_add_smartstart_node_to_network();
  /* Instead add our own callback for cmock nd it will basiclly save the
   * callback(completedFunc) funciton sent to
   * zwapi_add_smartstart_node_to_network(). This callback function will be
   * saved at on_node_add_started_Expect() which is used in later steps*/
  zwapi_add_smartstart_node_to_network_AddCallback(my_add_node_stub);
  zwave_controller_on_node_id_assigned_Expect(node_id, inclusion_protocol);
  zwave_controller_on_state_updated_Ignore();

  /* Call Network management */
  zwave_network_management_add_node_with_dsk(dsk,
                                             granted_keys,
                                             inclusion_protocol);

  contiki_test_helper_run(0);
  /* When we return here the state of NM should be NM_WAITING_FOR_ADD */
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  /*MOCK the serial api learn mode info and pass it to the callback */
  LEARN_INFO node_found = {ADD_NODE_STATUS_NODE_FOUND, node_id};
  add_node_callback_save(&node_found);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  node_found.bStatus = ADD_NODE_STATUS_ADDING_END_NODE;
  node_found.bSource = node_id;
  node_found.bLen
    = sizeof(node_add_nif_no_s2);  //nif_buffer and sample_nif are same
  node_found.pCmd = node_add_nif_no_s2;

  zwave_command_class_list_unpack_Expect(NULL,
                                         &node_found.pCmd[3],
                                         node_found.bLen - 3);
  zwave_command_class_list_unpack_ReturnThruPtr_node_info(sample_nif);
  zwave_command_class_list_unpack_IgnoreArg_node_info();
  add_node_callback_save(&node_found);
  zwave_controller_on_state_updated_Ignore();

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());

  /* Tell cmock to expect zwapi_add_node_to_network() call with arguments
   * (ADD_NODE_STOP, func) and ask it to return 0 on that call */
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, 0);
  zwapi_add_node_to_network_IgnoreArg_completedFunc();

  /* MOCK the serial api learn mode info again for next callback*/
  node_found.bStatus = ADD_NODE_STATUS_PROTOCOL_DONE;

  add_node_callback_save(&node_found);
  contiki_test_helper_run(0);

  /* Tell cmock to expect zwapi_add_node_to_network() call with arguments
   * (ADD_NODE_STOP, NULL) and ask it to return 0 on that call */
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, 0);
  /* MOCK the serial api learn mode info again for next callback*/
  node_found.bStatus = ADD_NODE_STATUS_DONE;

  /* Tell cmock to expect a call sec2_start_add_node()
   */

  zwave_s2_start_add_node_Expect(node_id);
  /* Instead ask cmock to call our own callback(my_sec2_inclusion_stub)  when
   * sec2_start_add_node() is called which will basiclly save the
   * callback(cb) funciton sent to
   * sec2_start_add_node(). This callback function will be
   * saved at my_sec2_inclusion_stub() which is used in later steps*/
  zwapi_get_protocol_info_ExpectAndReturn(node_id, 0, SL_STATUS_OK);
  zwapi_get_protocol_info_IgnoreArg_node_info_header();

  zwapi_node_info_header_t nodeInfo;
  nodeInfo.capability         = ZWAVE_NODE_INFO_LISTENING_PROTOCOL_ROUTING_MASK;
  nodeInfo.security           = ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_END_NODE_MASK;
  nodeInfo.reserved           = 0x44;
  nodeInfo.node_type.basic    = sample_nif->basic_device_class;
  nodeInfo.node_type.generic  = sample_nif->generic_device_class;
  nodeInfo.node_type.specific = sample_nif->specific_device_class;
  zwapi_get_protocol_info_ReturnThruPtr_node_info_header(&nodeInfo);
  add_node_callback_save(&node_found);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());

  // The S2 library will send a key requst, we expect the FSM to
  // grant the requested kyes.
  zwave_s2_key_grant_Expect(0x01, granted_keys, 0);
  my_sec2_inclusion_cb.on_keys_request(
    KEY_CLASS_S0 | KEY_CLASS_S2_UNAUTHENTICATED | KEY_CLASS_S2_AUTHENTICATED
      | KEY_CLASS_S2_ACCESS,
    0);
  zwave_controller_on_state_updated_Ignore();
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());

  // The S2 library will now send the DSK on the node beeing included
  // but with the the first two bytes blanked out.
}

void test_zwave_network_management_add_node_smartstart_wrong_dsk_self_destruct()
{
  smart_start_common();

  zwave_s2_dsk_accept_Expect(0, 0, 0);
  zwave_dsk_t reported_key;
  memcpy(reported_key, dsk, sizeof(zwave_dsk_t));
  reported_key[0] = 0;
  reported_key[1] = 0;
  reported_key[3] = 0;
  reported_key[4] = 0;

  /* core logic of the test in step below*/
  /* Post NM_EV_ADD_SECURITY_KEY_CHALLENGE to NM state machine with wrong dsk */
  my_sec2_inclusion_cb.on_dsk_challenge(granted_keys, 2, reported_key);

  /* Post NM_EV_SECURITY_DONE on NM state machine with kex_fail code */
  my_sec2_inclusion_cb.on_inclusion_complete(granted_keys, KEX_FAIL_AUTH);
  zwave_controller_on_state_updated_Ignore();
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());
  contiki_test_helper_run(0);

  //NM should now wait for self destruct
  TEST_ASSERT_EQUAL(NM_SEND_NOP, zwave_network_management_get_state());

  uint8_t nop_frame[1] = {0};
  zwave_tx_send_data_ExpectAndReturn(0,
                                     sizeof(nop_frame),
                                     nop_frame,
                                     0,
                                     0,
                                     0,
                                     0,
                                     SL_STATUS_OK);
  setup_mock_for_zwave_tx_send_data(SL_STATUS_OK);
  /* Setup zwapi_remove_failed_node() stuff. Also save the callback*/
  setup_zwapi_remove_failed_node_mock(1);

  /*Expect SmartStart enabled*/
  expect_smart_start_enable();

  /* Transition from NM_WAIT_FOR_SECURE_ADD to NM_WAIT_FOR_SECURE_ADD*/
  contiki_test_helper_run(SMART_START_SELF_DESTRUCT_TIMEOUT);

  my_zwave_tx_send_data_cb(TRANSMIT_COMPLETE_FAIL, 0, 0);
  contiki_test_helper_run(0);

  /* call the callback send to zwapi_remove_failed_node() with
   * ZW_FAILED_NODE_REMOVED*/
  remove_failed_node_callback_save(ZW_FAILED_NODE_REMOVED);

  // Check the zwave_controller_on_node_deleted() is called with the nodeID of the
  // node that is removed from the network.
  zwave_controller_on_node_deleted_Expect(node_id);

  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_add_node_self_destruct_nop_failed()
{
  smart_start_common();
  zwave_s2_dsk_accept_Expect(1, dsk, 2);

  zwave_dsk_t reported_key;
  memcpy(reported_key, dsk, sizeof(zwave_dsk_t));
  reported_key[0] = 0;
  reported_key[1] = 0;

  my_sec2_inclusion_cb.on_dsk_challenge(granted_keys, 2, reported_key);
  my_sec2_inclusion_cb.on_inclusion_complete(granted_keys, KEX_FAIL_KEX_KEY);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());

  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_SEND_NOP, zwave_network_management_get_state());

  //NM should be back in NM_IDLE state
  uint8_t nop_frame[1] = {0};
  zwave_tx_send_data_ExpectAndReturn(0,
                                     sizeof(nop_frame),
                                     nop_frame,
                                     0,
                                     0,
                                     0,
                                     0,
                                     SL_STATUS_FAIL);
  setup_mock_for_zwave_tx_send_data(SL_STATUS_FAIL);
  /* Setup zwapi_remove_failed_node() stuff. Also save the callback*/
  setup_zwapi_remove_failed_node_mock(1);
  contiki_test_helper_run(0);

  /*Expect SmartStart enabled*/
  expect_smart_start_enable();

  /* Transition from NM_WAIT_FOR_SECURE_ADD to NM_WAIT_FOR_SECURE_ADD*/
  contiki_test_helper_run(SMART_START_SELF_DESTRUCT_TIMEOUT);

  /* call the callback send to zwapi_remove_failed_node() with
   * ZW_FAILED_NODE_REMOVED*/
  remove_failed_node_callback_save(ZW_FAILED_NODE_REMOVED);

  // Check the zwave_controller_on_node_deleted() is called with the nodeID of the
  // node that is removed from the network.
  zwave_controller_on_node_deleted_Expect(node_id);

  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}
/* This test sends KEX_FAIL_KEX_KEY kex fail from on_inclusion_complete() callb
 * to test if node is self destructed */
void test_zwave_network_management_add_node_self_destruct()
{
  smart_start_common();
  zwave_s2_dsk_accept_Expect(1, dsk, 2);

  zwave_dsk_t reported_key;
  memcpy(reported_key, dsk, sizeof(zwave_dsk_t));
  reported_key[0] = 0;
  reported_key[1] = 0;

  my_sec2_inclusion_cb.on_dsk_challenge(granted_keys, 2, reported_key);
  my_sec2_inclusion_cb.on_inclusion_complete(granted_keys, KEX_FAIL_KEX_KEY);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());

  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_SEND_NOP, zwave_network_management_get_state());

  //NM should be back in NM_IDLE state
  uint8_t nop_frame[1] = {0};
  zwave_tx_send_data_ExpectAndReturn(0,
                                     sizeof(nop_frame),
                                     nop_frame,
                                     0,
                                     0,
                                     0,
                                     0,
                                     SL_STATUS_OK);
  setup_mock_for_zwave_tx_send_data(SL_STATUS_OK);
  /* Setup zwapi_remove_failed_node() stuff. Also save the callback*/
  setup_zwapi_remove_failed_node_mock(1);

  /*Expect SmartStart enabled*/
  expect_smart_start_enable();

  /* Transition from NM_WAIT_FOR_SECURE_ADD to NM_WAIT_FOR_SECURE_ADD*/
  contiki_test_helper_run(SMART_START_SELF_DESTRUCT_TIMEOUT);

  my_zwave_tx_send_data_cb(TRANSMIT_COMPLETE_FAIL, 0, 0);
  contiki_test_helper_run(0);

  /* call the callback send to zwapi_remove_failed_node() with
   * ZW_FAILED_NODE_REMOVED*/
  remove_failed_node_callback_save(ZW_FAILED_NODE_REMOVED);

  // Check the zwave_controller_on_node_deleted() is called with the nodeID of the
  // node that is removed from the network.
  zwave_controller_on_node_deleted_Expect(node_id);

  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/* This tests zwapi_remove_failed_node() not getting a callback from serialapi
 * and eventually timeout after SEND_DATA_TIMEOUT ms
 * to trigger NM_EV_TIMEOUT in
 * state: NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL
 */
void test_zwave_network_management_add_node_self_destruct_timeout()
{
  smart_start_common();
  zwave_s2_dsk_accept_Expect(1, dsk, 2);

  zwave_dsk_t reported_key;
  memcpy(reported_key, dsk, sizeof(zwave_dsk_t));
  reported_key[0] = 0;
  reported_key[1] = 0;

  my_sec2_inclusion_cb.on_dsk_challenge(granted_keys, 2, reported_key);
  my_sec2_inclusion_cb.on_inclusion_complete(granted_keys, KEX_FAIL_KEX_KEY);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_SEND_NOP, zwave_network_management_get_state());

  uint8_t nop_frame[1] = {0};
  zwave_tx_send_data_ExpectAndReturn(0,
                                     sizeof(nop_frame),
                                     nop_frame,
                                     0,
                                     0,
                                     0,
                                     0,
                                     SL_STATUS_OK);
  setup_mock_for_zwave_tx_send_data(SL_STATUS_OK);
  /* Setup zwapi_remove_failed_node() stuff. Also save the callback*/
  setup_zwapi_remove_failed_node_mock(0);

  /*Expect SmartStart enabled*/
  expect_smart_start_enable();

  /* Transition from NM_SEND_NOP to NM_WAIT_FOR_TX_TO_SELF_DESTRUCT*/
  contiki_test_helper_run(SMART_START_SELF_DESTRUCT_TIMEOUT);
  my_zwave_tx_send_data_cb(TRANSMIT_COMPLETE_FAIL, 0, 0);
  contiki_test_helper_run(0);
  zwave_controller_on_node_added_ExpectWithArray(
    SL_STATUS_OK,
    (zwave_node_info_t *)sample_nif,
    sizeof(nif_buffer),
    node_id,
    dsk,
    sizeof(dsk),
    granted_keys,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_UNKNOWN,
    PROTOCOL_ZWAVE);

  contiki_test_helper_run(SEND_DATA_TIMEOUT);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/* This function makes zwapi_remove_failed_node() return
 * !ZW_FAILED_NODE_REMOVE_STARTED which makes remove_self_destruct_status()
    * trigger NM_EV_REMOVE_FAILED_FAIL in state:
    * NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL to test that code path */
void test_zwave_network_management_add_node_self_destruct_failed()
{
  smart_start_common();
  zwave_s2_dsk_accept_Expect(1, dsk, 2);

  zwave_dsk_t reported_key;
  memcpy(reported_key, dsk, sizeof(zwave_dsk_t));
  reported_key[0] = 0;
  reported_key[1] = 0;

  my_sec2_inclusion_cb.on_dsk_challenge(granted_keys, 2, reported_key);
  my_sec2_inclusion_cb.on_inclusion_complete(granted_keys, KEX_FAIL_KEX_KEY);
  uint8_t nop_frame[1] = {0};
  zwave_tx_send_data_ExpectAndReturn(0,
                                     sizeof(nop_frame),
                                     nop_frame,
                                     0,
                                     0,
                                     0,
                                     0,
                                     SL_STATUS_OK);
  setup_mock_for_zwave_tx_send_data(SL_STATUS_OK);
  expect_smart_start_enable();
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());
  contiki_test_helper_run(0);

  /* Return ZW_FAILED_NODE_REMOVE_STARTED to zwapi_remove_failed_node() */
  /* No need of saving callback because the callback is called in NM state
   * machine itself */
  zwapi_remove_failed_node_ExpectAndReturn(node_id,
                                           0,
                                           !ZW_FAILED_NODE_REMOVE_STARTED);
  zwapi_remove_failed_node_IgnoreArg_completedFunc();
  zwave_controller_on_node_added_ExpectWithArray(
    SL_STATUS_OK,
    (zwave_node_info_t *)sample_nif,
    sizeof(nif_buffer),
    node_id,
    dsk,
    sizeof(dsk),
    granted_keys,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_UNKNOWN,
    PROTOCOL_ZWAVE);

  /* Transition from NM_WAIT_FOR_SECURE_ADD to NM_WAIT_FOR_SECURE_ADD*/
  contiki_test_helper_run(SMART_START_SELF_DESTRUCT_TIMEOUT);
  my_zwave_tx_send_data_cb(TRANSMIT_COMPLETE_FAIL, 0, 0);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/* This is happy case test */
void test_zwave_network_management_add_node_smartstart()
{
  smart_start_common();
  zwave_s2_dsk_accept_Expect(1, dsk, 2);
  zwave_dsk_t reported_key;
  memcpy(reported_key, dsk, sizeof(zwave_dsk_t));
  reported_key[0] = 0;
  reported_key[1] = 0;

  my_sec2_inclusion_cb.on_dsk_challenge(granted_keys, 2, reported_key);

  // Now the S2 library will complete the inclusion and the and we
  // expect to get our finalt inclusion complete callback, with the
  // full dsk and granted keys
  zwave_controller_on_node_added_ExpectWithArray(
    SL_STATUS_OK,
    (zwave_node_info_t *)sample_nif,
    sizeof(nif_buffer),
    node_id,
    dsk,
    sizeof(dsk),
    granted_keys,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,
    PROTOCOL_ZWAVE);

  //After inclusion we expect SM mode to be re-enabled.
  expect_smart_start_enable();

  my_sec2_inclusion_cb.on_inclusion_complete(
    granted_keys,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);

  //NM should be back in NM_IDLE state
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_smart_start_enable()
{
  expect_smart_start_enable();
  zwave_network_management_enable_smart_start_add_mode(true);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, 0, SL_STATUS_OK);
  zwave_network_management_enable_smart_start_add_mode(false);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/*
 * Set a set default where SmartStart was enabled before executing the command
 *
 */
void test_set_default()
{
  zwave_home_id_t home_id = 0xdeadbeef;
  zwave_node_id_t node_id = 0x1;

  //expect_smart_start_enable();
  zwave_network_management_enable_smart_start_add_mode(true);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  zwapi_set_default_ExpectAndReturn(0, SL_STATUS_OK);
  zwapi_set_default_IgnoreArg_completedFunc();
  zwapi_set_default_AddCallback(my_set_default_stub);

  zwave_network_management_set_default();
  contiki_test_helper_run(0);

  zwave_controller_on_reset_step_complete_Expect(5);
  zwapi_get_controller_capabilities_ExpectAndReturn(
    CONTROLLER_NODEID_SERVER_PRESENT);
  zwapi_memory_get_ids_ExpectAndReturn(0, 0, SL_STATUS_OK);
  zwapi_memory_get_ids_IgnoreArg_home_id();
  zwapi_memory_get_ids_IgnoreArg_node_id();
  zwapi_memory_get_ids_ReturnMemThruPtr_home_id(&home_id, sizeof(home_id));
  zwapi_memory_get_ids_ReturnMemThruPtr_node_id(&node_id, sizeof(node_id));
  zwave_controller_on_network_address_update_Expect(home_id, node_id);
  zwave_s2_create_new_network_keys_Expect();
  zwave_s2_network_init_Expect();

  //on_new_network()
  zwapi_memory_get_ids_ExpectAndReturn(0, 0, SL_STATUS_OK);
  zwapi_memory_get_ids_IgnoreArg_home_id();
  zwapi_memory_get_ids_IgnoreArg_node_id();
  zwapi_memory_get_ids_ReturnMemThruPtr_home_id(&home_id, sizeof(home_id));
  zwapi_memory_get_ids_ReturnMemThruPtr_node_id(&node_id, sizeof(node_id));

  zwapi_get_controller_capabilities_ExpectAndReturn(0xFF);
  zwave_s2_keystore_get_assigned_keys_ExpectAndReturn(0x87);

  zwave_controller_on_new_network_entered_Expect(
    home_id,
    node_id,
    0x87,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);

  //expect_smart_start_enable();

  my_set_default_callbacks_save();
  contiki_test_helper_run(0);
}

// Test the remove node capability
void test_zwave_network_management_remove_node()
{
  zwave_node_id_t node_id = 0x01;
  LEARN_INFO sample_remove_node_report;
  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  // Mocking the zwapi_remove_node_from_network function call
  zwapi_remove_node_from_network_ExpectAndReturn(
    (REMOVE_NODE_ANY | REMOVE_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_remove_node_from_network
  zwapi_remove_node_from_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();
  // Mocking the callback function that is triggered when zwapi_remove_node_from_network is called.
  // This is done by saving the callback(completedFunc) function sent to zwapi_remove_node_from_network().
  zwapi_remove_node_from_network_AddCallback(remove_node_stub);
  zwave_network_management_remove_node();
  contiki_test_helper_run(0);
  // Check the Network Management state is 'Removing Node' state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_NODE_REMOVAL,
                    zwave_network_management_get_state());

  // Check the zwave_controller_on_node_deleted() is called with the nodeID of the
  // node that is removed from the network.
  zwave_controller_on_node_deleted_Expect(node_id);
  // Mocking the Z-Wave API to initiate an exclusion on a given NodeID
  sample_remove_node_report.bStatus = REMOVE_NODE_STATUS_REMOVING_END_NODE;
  sample_remove_node_report.bSource = node_id;
  remove_node_callback_save(&sample_remove_node_report);
  contiki_test_helper_run(0);

  // Mocking the Network Exclusion Done Status from the Z-Wave API
  sample_remove_node_report.bStatus = REMOVE_NODE_STATUS_DONE;
  sample_remove_node_report.bSource = 0;  // Z-Wave API does not pass any NodeID
  remove_node_callback_save(&sample_remove_node_report);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  contiki_test_helper_run(0);
  // Check the Network Management state is back to Idle state
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/*
 * A test case for Remove Node timeout.
 * If the Network managemnt state machine did not receive any remove node status indication
 * after calling zwapi_remove_node_from_network(), it should timeout after 60 seconds.
 */
void test_zwave_network_management_remove_node_timeout()
{
  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(
    (REMOVE_NODE_ANY | REMOVE_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_remove_node_from_network
  zwapi_remove_node_from_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();
  zwapi_remove_node_from_network_AddCallback(remove_node_stub);
  zwave_network_management_remove_node();
  // Fire the remove node event
  contiki_test_helper_run(0);
  // check the Network Management state is 'Removing Node' state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_NODE_REMOVAL,
                    zwave_network_management_get_state());

  // Trigger the contiki event system with 40 seconds progress time.
  // At this stage the Network Managment shall still be in 'NM_WAITING_FOR_NODE_REMOVAL' state
  contiki_test_helper_run(40000);
  // Check the Network Management state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_NODE_REMOVAL,
                    zwave_network_management_get_state());

  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  // Trigger the timeout of the Network Managemnt 'NM_WAITING_FOR_NODE_REMOVAL' state.
  // At this stage the Network Managemnt shall be back to Idle state, since the progress
  // time interval is higher than the Node Remove timeout interval (i.e, 60 seconds).
  contiki_test_helper_run(70000);
  // check the Network Managemnt state is back to IDLE
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

// Test case for aborting the node remove network management operation
void test_zwave_network_managemnt_remove_node_abort()
{
  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  zwapi_remove_node_from_network_ExpectAndReturn(
    (REMOVE_NODE_ANY | REMOVE_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_remove_node_from_network
  zwapi_remove_node_from_network_IgnoreArg_completedFunc();
  zwapi_remove_node_from_network_AddCallback(remove_node_stub);
  zwave_controller_on_state_updated_Ignore();
  zwave_network_management_remove_node();
  // Fire the remove node event
  contiki_test_helper_run(0);
  // check the Network Management state is 'Removing Node' state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_NODE_REMOVAL,
                    zwave_network_management_get_state());

  // Mocking zwapi_remove_node_from_network function call, since it wil be called when the
  // node remove operation is aborted.
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  // Aborting the remove node network management operation
  zwave_network_management_abort();
  // Trigger the contiki event system with 40 seconds progress time.
  // At this stage the Network Managment shall still be in 'NM_WAITING_FOR_NODE_REMOVAL' state
  contiki_test_helper_run(40000);
  // check the Network Management state is IDLE after abort operation
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_failed_node_remove_failed_as_nop_success()
{
  zwave_node_id_t node_id = 0x01;
  uint8_t nop_frame[1]    = {0};
  zwave_controller_on_state_updated_Ignore();
  zwave_tx_send_data_ExpectAndReturn(0,
                                     sizeof(nop_frame),
                                     nop_frame,
                                     0,
                                     0,
                                     0,
                                     0,
                                     SL_STATUS_OK);
  setup_mock_for_zwave_tx_send_data(SL_STATUS_OK);
  zwave_network_management_remove_failed(node_id);
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(NM_FAILED_NODE_REMOVE,
                    zwave_network_management_get_state());
  contiki_test_helper_run(0);

  zwave_controller_on_state_updated_Ignore();

  my_zwave_tx_send_data_cb(TRANSMIT_COMPLETE_OK, 0, 0);  //Call nop_send_done()

  contiki_test_helper_run(0);
  // Check the Network Management state is 'Failed node removing' state
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void zwave_network_management_failed_node_remove_fail_common()
{
  zwave_node_id_t node_id = 0x01;
  uint8_t nop_frame[1]    = {0};
  zwave_controller_on_state_updated_Ignore();
  zwave_tx_send_data_ExpectAndReturn(0,
                                     sizeof(nop_frame),
                                     nop_frame,
                                     0,
                                     0,
                                     0,
                                     0,
                                     SL_STATUS_OK);
  setup_mock_for_zwave_tx_send_data(SL_STATUS_OK);
  zwave_network_management_remove_failed(node_id);
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(NM_FAILED_NODE_REMOVE,
                    zwave_network_management_get_state());

  // Mocking the zwapi_remove_node_from_network function call
  zwapi_remove_failed_node_ExpectAndReturn(node_id,
                                           0,
                                           ZW_FAILED_NODE_REMOVE_STARTED);
  // Ignoring the completedFunc argument of zwapi_remove_node_from_network
  zwapi_remove_failed_node_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();
  // Mocking the callback function that is triggered when zwapi_remove_node_from_network is called.
  // This is done by saving the callback(completedFunc) function sent to zwapi_remove_node_from_network().
  zwapi_remove_failed_node_AddCallback(failed_node_remove_stub);

  my_zwave_tx_send_data_cb(TRANSMIT_COMPLETE_FAIL,
                           0,
                           0);  //Call nop_send_done()
  contiki_test_helper_run(0);
  // Check the Network Management state is 'Failed node removing' state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_FAILED_NODE_REMOVAL,
                    zwave_network_management_get_state());
}

// Test the failed node remove capability
void test_zwave_network_management_failed_node_remove()
{
  zwave_node_id_t node_id = 0x01;

  zwave_network_management_failed_node_remove_fail_common();
  // Check the zwave_controller_on_node_deleted() is called with the nodeID of the
  // node that is removed from the network.
  zwave_controller_on_node_deleted_Expect(node_id);
  // Mocking the Z-Wave API to initiate an exclusion on a given NodeID
  failed_node_remove_callback_save(ZW_FAILED_NODE_REMOVED);
  contiki_test_helper_run(0);

  // Check the Network Management state is back to Idle state
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

// Test the failed node remove fail with callback status
// ZW_FAILED_NODE_NOT_REMOVED
void test_zwave_network_management_failed_node_remove_fail()
{
  zwave_network_management_failed_node_remove_fail_common();
  zwave_controller_on_error_Expect(
    ZWAVE_NETWORK_MANAGEMENT_ERROR_FAILED_NODE_REMOVE_FAIL);
  // Mocking the Z-Wave API to initiate an exclusion on a given NodeID
  failed_node_remove_callback_save(ZW_FAILED_NODE_NOT_REMOVED);
  contiki_test_helper_run(0);

  // Check the Network Management state is back to Idle state
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

// Test the failed node remove fail with callback status ZW_NODE_OK
void test_zwave_network_management_failed_node_remove_fail_zwave_node_ok()
{
  zwave_network_management_failed_node_remove_fail_common();
  zwave_controller_on_error_Expect(
    ZWAVE_NETWORK_MANAGEMENT_ERROR_FAILED_NODE_REMOVE_FAIL);
  // Mocking the Z-Wave API to initiate an exclusion on a given NodeID
  failed_node_remove_callback_save(ZW_NODE_OK);
  contiki_test_helper_run(0);

  // Check the Network Management state is back to Idle state
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

// Test the failed node remove fail with callback status
// ZW_FAILED_NODE_NOT_FOUND
void test_zwave_network_management_failed_node_remove_fail_node_not_found()
{
  zwave_network_management_failed_node_remove_fail_common();

  zwave_controller_on_error_Expect(
    ZWAVE_NETWORK_MANAGEMENT_ERROR_FAILED_NODE_REMOVE_FAIL);
  // Mocking the Z-Wave API to initiate an exclusion on a given NodeID
  failed_node_remove_callback_save(ZW_FAILED_NODE_NOT_FOUND);
  contiki_test_helper_run(0);

  // Check the Network Management state is back to Idle state
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}
//
/// A test for all NM state changes for S0 inclusion failure
void test_zwave_network_management_s0_add_controller_failed()
{
  node_id                           = 0x44;
  LEARN_INFO sample_add_node_report = {ADD_NODE_STATUS_LEARN_READY, node_id};

  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  // Mocking the zwapi_add_node_from_network function call
  zwapi_add_node_to_network_ExpectAndReturn(
    (ADD_NODE_ANY | ADD_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_add_node_from_network
  zwapi_add_node_to_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();

  zwapi_add_node_to_network_AddCallback(my_s2_add_node_stub);

  zwave_network_management_add_node();
  contiki_test_helper_run(0);
  // check the Network Management state is 'Add Node' state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_NODE_FOUND;
  sample_add_node_report.bSource = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_ADDING_CONTROLLER;
  sample_add_node_report.pCmd    = node_add_nif_with_s2;
  sample_add_node_report.bSource = node_id;
  sample_add_node_report.bLen    = sizeof(node_add_nif_with_s2);
  my_s2_add_node_callback_save(&sample_add_node_report);

  sample_nif->basic_device_class    = BASIC_TYPE_END_NODE;
  sample_nif->command_class_list[0] = COMMAND_CLASS_ZWAVEPLUS_INFO;
  sample_nif->command_class_list[1] = COMMAND_CLASS_SECURITY;  //support only S0

  zwave_command_class_list_unpack_Expect(NULL,
                                         &sample_add_node_report.pCmd[3],
                                         sample_add_node_report.bLen - 3);
  zwave_command_class_list_unpack_ReturnThruPtr_node_info(sample_nif);
  zwave_command_class_list_unpack_IgnoreArg_node_info();

  zwave_controller_on_node_id_assigned_Expect(node_id, PROTOCOL_ZWAVE);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_DONE;
  sample_add_node_report.bSource = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);

  zwapi_add_node_to_network_ExpectAndReturn((ADD_NODE_STOP), 0, SL_STATUS_OK);
  zwapi_add_node_to_network_IgnoreArg_completedFunc();

  zwapi_get_protocol_info_ExpectAndReturn(node_id, NULL, SL_STATUS_OK),
    zwapi_get_protocol_info_IgnoreArg_node_info_header();
  zwave_s0_start_bootstrapping_Expect(node_id, 1);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());
  my_sec0_inclusion_cb(0, ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_SCHEME);
  zwave_controller_on_node_added_ExpectWithArray(
    SL_STATUS_OK,
    (zwave_node_info_t *)sample_nif,
    sizeof(nif_buffer),
    node_id,
    dsk,
    sizeof(dsk),
    0,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_SCHEME,
    PROTOCOL_ZWAVE);
  zwave_controller_on_node_added_IgnoreArg_nif();
  zwave_controller_on_node_added_IgnoreArg_dsk();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_stop_add_mode()
{
  node_id = 0x44;

  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  // Mocking the zwapi_add_node_from_network function call
  zwapi_add_node_to_network_ExpectAndReturn(
    (ADD_NODE_ANY | ADD_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_add_node_from_network
  zwapi_add_node_to_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();

  zwapi_add_node_to_network_AddCallback(my_s2_add_node_stub);

  zwave_network_management_add_node();
  contiki_test_helper_run(0);
  // check the Network Management state is 'Add Node' state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_network_management_abort());
  zwapi_add_node_to_network_ExpectAndReturn((ADD_NODE_STOP), 0, SL_STATUS_OK);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/// A test for all NM state changes for S0 inclusion of a controller
void test_zwave_network_management_s0_add_controller()
{
  node_id                           = 0x44;
  LEARN_INFO sample_add_node_report = {ADD_NODE_STATUS_LEARN_READY, node_id};

  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  // Mocking the zwapi_add_node_from_network function call
  zwapi_add_node_to_network_ExpectAndReturn(
    (ADD_NODE_ANY | ADD_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_add_node_from_network
  zwapi_add_node_to_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();

  zwapi_add_node_to_network_AddCallback(my_s2_add_node_stub);

  zwave_network_management_add_node();
  contiki_test_helper_run(0);
  // check the Network Management state is 'Add Node' state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_NODE_FOUND;
  sample_add_node_report.bSource = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_ADDING_CONTROLLER;
  sample_add_node_report.pCmd    = node_add_nif_with_s2;
  sample_add_node_report.bSource = node_id;
  sample_add_node_report.bLen    = sizeof(node_add_nif_with_s2);
  my_s2_add_node_callback_save(&sample_add_node_report);

  sample_nif->basic_device_class    = BASIC_TYPE_END_NODE;
  sample_nif->command_class_list[0] = COMMAND_CLASS_ZWAVEPLUS_INFO;
  sample_nif->command_class_list[1] = COMMAND_CLASS_SECURITY;  //support only S0

  zwave_command_class_list_unpack_Expect(NULL,
                                         &sample_add_node_report.pCmd[3],
                                         sample_add_node_report.bLen - 3);
  zwave_command_class_list_unpack_ReturnThruPtr_node_info(sample_nif);
  zwave_command_class_list_unpack_IgnoreArg_node_info();

  zwave_controller_on_node_id_assigned_Expect(node_id, PROTOCOL_ZWAVE);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_DONE;
  sample_add_node_report.bSource = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);

  zwapi_add_node_to_network_ExpectAndReturn((ADD_NODE_STOP), 0, SL_STATUS_OK);
  zwapi_add_node_to_network_IgnoreArg_completedFunc();

  zwapi_get_protocol_info_ExpectAndReturn(node_id, NULL, SL_STATUS_OK),
    zwapi_get_protocol_info_IgnoreArg_node_info_header();
  zwave_s0_start_bootstrapping_Expect(node_id, true);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());
  my_sec0_inclusion_cb(KEY_CLASS_S0, ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
  zwave_controller_on_node_added_ExpectWithArray(
    SL_STATUS_OK,
    (zwave_node_info_t *)sample_nif,
    sizeof(nif_buffer),
    node_id,
    dsk,
    sizeof(dsk),
    KEY_CLASS_S0,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,
    PROTOCOL_ZWAVE);
  zwave_controller_on_node_added_IgnoreArg_nif();
  zwave_controller_on_node_added_IgnoreArg_dsk();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/// A test for all NM state changes for S0 inclusion of an end node
void test_zwave_network_management_s0_add_end_node()
{
  node_id                           = 0x44;
  LEARN_INFO sample_add_node_report = {ADD_NODE_STATUS_LEARN_READY, node_id};

  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  // Mocking the zwapi_add_node_from_network function call
  zwapi_add_node_to_network_ExpectAndReturn(
    (ADD_NODE_ANY | ADD_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_add_node_from_network
  zwapi_add_node_to_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();

  zwapi_add_node_to_network_AddCallback(my_s2_add_node_stub);

  zwave_network_management_add_node();
  contiki_test_helper_run(0);
  // check the Network Management state is 'Add Node' state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_NODE_FOUND;
  sample_add_node_report.bSource = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_ADDING_END_NODE;
  sample_add_node_report.pCmd    = node_add_nif_with_s2;
  sample_add_node_report.bSource = node_id;
  sample_add_node_report.bLen    = sizeof(node_add_nif_with_s2);
  my_s2_add_node_callback_save(&sample_add_node_report);

  sample_nif->basic_device_class    = BASIC_TYPE_END_NODE;
  sample_nif->command_class_list[0] = COMMAND_CLASS_ZWAVEPLUS_INFO;
  sample_nif->command_class_list[1] = COMMAND_CLASS_SECURITY;  //support only S0

  zwave_command_class_list_unpack_Expect(NULL,
                                         &sample_add_node_report.pCmd[3],
                                         sample_add_node_report.bLen - 3);
  zwave_command_class_list_unpack_ReturnThruPtr_node_info(sample_nif);
  zwave_command_class_list_unpack_IgnoreArg_node_info();

  zwave_controller_on_node_id_assigned_Expect(node_id, PROTOCOL_ZWAVE);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_DONE;
  sample_add_node_report.bSource = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);

  zwapi_add_node_to_network_ExpectAndReturn((ADD_NODE_STOP), 0, SL_STATUS_OK);
  zwapi_add_node_to_network_IgnoreArg_completedFunc();

  zwapi_get_protocol_info_ExpectAndReturn(node_id, NULL, SL_STATUS_OK),
    zwapi_get_protocol_info_IgnoreArg_node_info_header();
  zwave_s0_start_bootstrapping_Expect(node_id, false);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());
  my_sec0_inclusion_cb(KEY_CLASS_S0, ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
  zwave_controller_on_node_added_ExpectWithArray(
    SL_STATUS_OK,
    (zwave_node_info_t *)sample_nif,
    sizeof(nif_buffer),
    node_id,
    dsk,
    sizeof(dsk),
    KEY_CLASS_S0,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,
    PROTOCOL_ZWAVE);
  zwave_controller_on_node_added_IgnoreArg_nif();
  zwave_controller_on_node_added_IgnoreArg_dsk();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}
//
// A test for all NM state changes for S2 inclusion
void test_zwave_network_management_add_node_with_same_dsk_in_smart_start_list()
{
  node_id                           = 0x44;
  LEARN_INFO sample_add_node_report = {ADD_NODE_STATUS_LEARN_READY, node_id};

  for (int i = 0; i < sizeof(dsk); i++) {
    dsk[i] = i;
  }

  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  // Mocking the zwapi_add_node_from_network function call
  zwapi_add_node_to_network_ExpectAndReturn(
    (ADD_NODE_ANY | ADD_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_add_node_from_network
  zwapi_add_node_to_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();

  zwapi_add_node_to_network_AddCallback(my_s2_add_node_stub);

  zwave_network_management_add_node();
  contiki_test_helper_run(0);
  // check the Network Management state is 'Add Node' state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_NODE_FOUND;
  sample_add_node_report.bSource = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_ADDING_END_NODE;
  sample_add_node_report.pCmd    = node_add_nif_with_s2;
  sample_add_node_report.bSource = node_id;
  sample_add_node_report.bLen    = sizeof(node_add_nif_with_s2);
  my_s2_add_node_callback_save(&sample_add_node_report);

  sample_nif->basic_device_class    = BASIC_TYPE_END_NODE;
  sample_nif->command_class_list[0] = COMMAND_CLASS_ZWAVEPLUS_INFO;
  sample_nif->command_class_list[1] = COMMAND_CLASS_SECURITY_2;

  zwave_command_class_list_unpack_Expect(NULL,
                                         &sample_add_node_report.pCmd[3],
                                         sample_add_node_report.bLen - 3);
  zwave_command_class_list_unpack_ReturnThruPtr_node_info(sample_nif);
  zwave_command_class_list_unpack_IgnoreArg_node_info();

  zwave_controller_on_node_id_assigned_Expect(node_id, PROTOCOL_ZWAVE);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_DONE;
  sample_add_node_report.bSource = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);

  zwapi_add_node_to_network_ExpectAndReturn((ADD_NODE_STOP), 0, SL_STATUS_OK);
  zwapi_add_node_to_network_IgnoreArg_completedFunc();

  zwapi_get_protocol_info_ExpectAndReturn(node_id, NULL, SL_STATUS_OK),
    zwapi_get_protocol_info_IgnoreArg_node_info_header();
  zwave_s2_start_add_node_Expect(node_id);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());

  // Expect zwave_controller_on_keys_report() being called after arrival of
  // kex report
  uint8_t key_request = KEY_CLASS_S0 | KEY_CLASS_S2_UNAUTHENTICATED
                        | KEY_CLASS_S2_AUTHENTICATED | KEY_CLASS_S2_ACCESS;
  zwave_controller_on_keys_report_Expect(0, key_request);
  zwave_controller_on_keys_report_IgnoreArg_csa();
  // Fire NM_EV_ADD_SECURITY_REQ_KEYS (That keys report has arrived from other side)
  my_sec2_inclusion_cb.on_keys_request(key_request, 0);
  /* ----------------------------------------------------*/

  zwave_controller_on_state_updated_Ignore();
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());

  //expect zwave_s2_key_grant() called
  zwave_s2_key_grant_Expect(1, key_request, 0);

  // Fire NM_EV_ADD_SECURITY_KEYS_SET (Grant keys)
  zwave_network_management_keys_set(1, 0, key_request);
  contiki_test_helper_run(0);

  /* ----------------------------------------------------*/
  find_dsk_obfuscated_bytes_from_smart_start_list_ExpectAndReturn(0, 0, true);
  find_dsk_obfuscated_bytes_from_smart_start_list_IgnoreArg_dsk();
  find_dsk_obfuscated_bytes_from_smart_start_list_IgnoreArg_obfuscated_bytes();
  //Fire NM_EV_ADD_SECURITY_KEY_CHALLENGE
  my_sec2_inclusion_cb.on_dsk_challenge(granted_keys, 2, dsk);
  zwave_s2_dsk_accept_Expect(1, dsk, 2);
  contiki_test_helper_run(0);

  /* ----------------------------------------------------*/
  zwave_s2_dsk_accept_Expect(1, dsk, 2);
  // Fire NM_EV_ADD_SECURITY_DSK_SET
  zwave_network_management_dsk_set(dsk);
  contiki_test_helper_run(0);

  /* ----------------------------------------------------*/
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());
  printf("----");
  // Now the S2 library will complete the inclusion and the and we
  // expect to get our finalt inclusion complete callback, with the
  // full dsk and granted keys
  zwave_controller_on_node_added_ExpectWithArray(
    SL_STATUS_OK,
    (zwave_node_info_t *)node_add_nif_with_s2,
    sizeof(node_add_nif_with_s2),
    node_id,
    dsk,
    sizeof(dsk),
    granted_keys,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,
    PROTOCOL_ZWAVE);

  zwave_controller_on_node_added_IgnoreArg_nif();
  // Fire NM_EV_SECURITY_DONE
  my_sec2_inclusion_cb.on_inclusion_complete(
    granted_keys,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
  contiki_test_helper_run(0);
  /* ----------------------------------------------------*/
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}
// A test for all NM state changes for S2 inclusion
void test_zwave_network_management_add_node()
{
  node_id                           = 0x44;
  LEARN_INFO sample_add_node_report = {ADD_NODE_STATUS_LEARN_READY, node_id};

  for (int i = 0; i < sizeof(dsk); i++) {
    dsk[i] = i;
  }

  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  // Mocking the zwapi_add_node_from_network function call
  zwapi_add_node_to_network_ExpectAndReturn(
    (ADD_NODE_ANY | ADD_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_add_node_from_network
  zwapi_add_node_to_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();

  zwapi_add_node_to_network_AddCallback(my_s2_add_node_stub);

  zwave_network_management_add_node();
  contiki_test_helper_run(0);
  // check the Network Management state is 'Add Node' state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_NODE_FOUND;
  sample_add_node_report.bSource = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_ADDING_END_NODE;
  sample_add_node_report.pCmd    = node_add_nif_with_s2;
  sample_add_node_report.bSource = node_id;
  sample_add_node_report.bLen    = sizeof(node_add_nif_with_s2);
  my_s2_add_node_callback_save(&sample_add_node_report);

  sample_nif->basic_device_class    = BASIC_TYPE_END_NODE;
  sample_nif->command_class_list[0] = COMMAND_CLASS_ZWAVEPLUS_INFO;
  sample_nif->command_class_list[1] = COMMAND_CLASS_SECURITY_2;

  zwave_command_class_list_unpack_Expect(NULL,
                                         &sample_add_node_report.pCmd[3],
                                         sample_add_node_report.bLen - 3);
  zwave_command_class_list_unpack_ReturnThruPtr_node_info(sample_nif);
  zwave_command_class_list_unpack_IgnoreArg_node_info();

  zwave_controller_on_node_id_assigned_Expect(node_id, PROTOCOL_ZWAVE);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_DONE;
  sample_add_node_report.bSource = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);

  zwapi_add_node_to_network_ExpectAndReturn((ADD_NODE_STOP), 0, SL_STATUS_OK);
  zwapi_add_node_to_network_IgnoreArg_completedFunc();

  zwapi_get_protocol_info_ExpectAndReturn(node_id, NULL, SL_STATUS_OK),
    zwapi_get_protocol_info_IgnoreArg_node_info_header();
  zwave_s2_start_add_node_Expect(node_id);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());

  // Expect zwave_controller_on_keys_report() being called after arrival of
  // kex report
  uint8_t key_request = KEY_CLASS_S0 | KEY_CLASS_S2_UNAUTHENTICATED
                        | KEY_CLASS_S2_AUTHENTICATED | KEY_CLASS_S2_ACCESS;
  zwave_controller_on_keys_report_Expect(0, key_request);
  zwave_controller_on_keys_report_IgnoreArg_csa();
  // Fire NM_EV_ADD_SECURITY_REQ_KEYS (That keys report has arrived from other side)
  my_sec2_inclusion_cb.on_keys_request(key_request, 0);
  /* ----------------------------------------------------*/

  zwave_controller_on_state_updated_Ignore();
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());

  //expect zwave_s2_key_grant() called
  zwave_s2_key_grant_Expect(1, key_request, 0);

  // Fire NM_EV_ADD_SECURITY_KEYS_SET (Grant keys)
  zwave_network_management_keys_set(1, 0, key_request);
  contiki_test_helper_run(0);

  /* ----------------------------------------------------*/
  zwave_controller_on_dsk_report_Expect(2, dsk, 0x87);
  find_dsk_obfuscated_bytes_from_smart_start_list_ExpectAndReturn(0, 0, false);
  find_dsk_obfuscated_bytes_from_smart_start_list_IgnoreArg_dsk();
  find_dsk_obfuscated_bytes_from_smart_start_list_IgnoreArg_obfuscated_bytes();
  //Fire NM_EV_ADD_SECURITY_KEY_CHALLENGE
  my_sec2_inclusion_cb.on_dsk_challenge(granted_keys, 2, dsk);
  contiki_test_helper_run(0);

  /* ----------------------------------------------------*/
  zwave_s2_dsk_accept_Expect(1, dsk, 2);
  // Fire NM_EV_ADD_SECURITY_DSK_SET
  zwave_network_management_dsk_set(dsk);
  contiki_test_helper_run(0);

  /* ----------------------------------------------------*/
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());
  printf("----");
  // Now the S2 library will complete the inclusion and the and we
  // expect to get our finalt inclusion complete callback, with the
  // full dsk and granted keys
  zwave_controller_on_node_added_ExpectWithArray(
    SL_STATUS_OK,
    (zwave_node_info_t *)node_add_nif_with_s2,
    sizeof(node_add_nif_with_s2),
    node_id,
    dsk,
    sizeof(dsk),
    granted_keys,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,
    PROTOCOL_ZWAVE);

  zwave_controller_on_node_added_IgnoreArg_nif();
  // Fire NM_EV_SECURITY_DONE
  my_sec2_inclusion_cb.on_inclusion_complete(
    granted_keys,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
  contiki_test_helper_run(0);
  /* ----------------------------------------------------*/
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/* A inclusion timeout test which tests if the NM process will timeout
 * waiting for ADD_NODE_STATUS_NODE_FOUND if this event is not arrived within 60 seconds.
 * Timeout duration: 60 seconds; See Application Programmers guide: ZW_AddNodeToNetwork() section */
void test_zwave_network_management_add_node_timeout()
{
  /* Check if the Network management state is idle */
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  // Mocking the zwapi_add_node_from_network function call
  zwapi_add_node_to_network_ExpectAndReturn(
    (ADD_NODE_ANY | ADD_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_add_node_from_network
  zwapi_add_node_to_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();

  zwapi_add_node_to_network_AddCallback(my_s2_add_node_stub);

  zwave_network_management_add_node();
  contiki_test_helper_run(0);

  /* When we return here the state of NM should be NM_WAITING_FOR_ADD */
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  // Tell cmock to expect (ADD_NODE_STOP)
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, 0);

  //Tell cmock to expect error
  zwave_controller_on_error_Expect(
    ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL);

  //Fire the timeout event
  contiki_test_helper_run(ADD_REMOVE_TIMEOUT * 10);

  //NM should be back in NM_IDLE state
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/* A inclusion timeout test which tests if the NM process will timeout
 * waiting for ADD_NODE_STATUS_ADDING_END_NODE event if this event is not arrived within a time period
 * less than 60 seconds after receiving the ADD_NODE_STATUS_NODE_FOUND status.*/
void test_zwave_network_management_add_node_end_node_found_timeout()
{
  /* Check if the Network management state is idle */
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  // Mocking the zwapi_add_node_from_network function call
  zwapi_add_node_to_network_ExpectAndReturn(
    (ADD_NODE_ANY | ADD_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_add_node_from_network
  zwapi_add_node_to_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();

  zwapi_add_node_to_network_AddCallback(my_s2_add_node_stub);

  zwave_network_management_add_node();
  contiki_test_helper_run(0);

  /* When we return here the state of NM should be NM_WAITING_FOR_ADD */
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  zwave_node_id_t node_id           = 0x66;
  LEARN_INFO sample_add_node_report = {ADD_NODE_STATUS_NODE_FOUND, node_id};
  my_s2_add_node_callback_save(&sample_add_node_report);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  // Tell cmock to expect (ADD_NODE_STOP_FAILED)
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP_FAILED, NULL, 0);

  //Tell cmock to expect error
  zwave_controller_on_error_Expect(
    ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL);

  //Fire the timeout event
  contiki_test_helper_run(CLOCK_SECOND * 60);

  //NM should be back in NM_IDLE state
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

/// test for wrong missing dsk input
void test_zwave_network_management_add_node_wrong_dsk_input()
{
  //
  node_id                           = 0x44;
  LEARN_INFO sample_add_node_report = {ADD_NODE_STATUS_LEARN_READY, node_id};

  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  // Mocking the zwapi_add_node_from_network function call
  zwapi_add_node_to_network_ExpectAndReturn(
    (ADD_NODE_ANY | ADD_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_add_node_from_network
  zwapi_add_node_to_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();

  zwapi_add_node_to_network_AddCallback(my_s2_add_node_stub);

  zwave_network_management_add_node();
  contiki_test_helper_run(0);
  // check the Network Management state is 'Add Node' state
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_NODE_FOUND;
  my_s2_add_node_callback_save(&sample_add_node_report);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_ADDING_END_NODE;
  sample_add_node_report.pCmd    = node_add_nif_with_s2;
  sample_add_node_report.bLen    = sizeof(node_add_nif_with_s2);
  my_s2_add_node_callback_save(&sample_add_node_report);

  sample_nif->basic_device_class    = BASIC_TYPE_END_NODE;
  sample_nif->command_class_list[0] = COMMAND_CLASS_ZWAVEPLUS_INFO;
  sample_nif->command_class_list[1] = COMMAND_CLASS_SECURITY_2;

  zwave_command_class_list_unpack_Expect(NULL,
                                         &sample_add_node_report.pCmd[3],
                                         sample_add_node_report.bLen - 3);
  zwave_command_class_list_unpack_ReturnThruPtr_node_info(sample_nif);
  zwave_command_class_list_unpack_IgnoreArg_node_info();

  zwave_controller_on_node_id_assigned_Expect(node_id, PROTOCOL_ZWAVE);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_DONE;
  my_s2_add_node_callback_save(&sample_add_node_report);

  zwapi_add_node_to_network_ExpectAndReturn((ADD_NODE_STOP), 0, SL_STATUS_OK);
  zwapi_add_node_to_network_IgnoreArg_completedFunc();

  zwapi_get_protocol_info_ExpectAndReturn(node_id, NULL, SL_STATUS_OK),
    zwapi_get_protocol_info_IgnoreArg_node_info_header();
  zwave_s2_start_add_node_Expect(node_id);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());
  //

  zwave_s2_dsk_accept_Expect(1, dsk, 2);
  zwave_dsk_t reported_dsk;
  memcpy(reported_dsk, dsk, sizeof(zwave_dsk_t));
  reported_dsk[0] = 1;
  reported_dsk[1] = 1;

  find_dsk_obfuscated_bytes_from_smart_start_list_ExpectAndReturn(0, 0, false);
  find_dsk_obfuscated_bytes_from_smart_start_list_IgnoreArg_dsk();
  find_dsk_obfuscated_bytes_from_smart_start_list_IgnoreArg_obfuscated_bytes();
  // Post NM_EV_ADD_SECURITY_KEY_CHALLENGE to NM state machine with wrong dsk
  my_sec2_inclusion_cb.on_dsk_challenge(granted_keys, 2, reported_dsk);
  zwave_network_management_dsk_set(dsk);
  //Post NM_EV_SECURITY_DONE on NM state machine with DSK missmatch kex_fail code
  my_sec2_inclusion_cb.on_inclusion_complete(granted_keys, KEX_FAIL_DECRYPT);
  zwave_controller_on_dsk_report_Expect(2, reported_dsk, 0x00);

  zwave_controller_on_node_added_ExpectWithArray(
    SL_STATUS_OK,
    (zwave_node_info_t *)sample_nif,
    sizeof(nif_buffer),
    node_id,
    dsk,
    sizeof(dsk),
    granted_keys,
    KEX_FAIL_DECRYPT,
    PROTOCOL_ZWAVE);
  zwave_controller_on_node_added_IgnoreArg_nif();
  zwave_controller_on_node_added_IgnoreArg_dsk();

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

// Test failure due to a low level protocol failure
void test_zwave_network_management_learn_mode_inclusion_fail()
{
  LEARN_INFO test_learn_mode_info;

  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_NWI, NULL, SL_STATUS_OK);
  zwapi_set_learn_mode_IgnoreArg_completedFunc();
  zwapi_set_learn_mode_AddCallback(my_zwapi_set_learn_mode_stub);
  zwave_controller_on_state_updated_Ignore();
  zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());

  test_learn_mode_info.bStatus = LEARN_MODE_STARTED;
  test_learn_mode_info.bSource = 1;
  my_zwapi_set_learn_mode_cb_save(&test_learn_mode_info);

  zwave_s0_start_learn_mode_Expect(1);
  zwave_s2_start_learn_mode_Expect(1);
  contiki_test_helper_run(0);

  test_learn_mode_info.bStatus = LEARN_MODE_FAILED;
  test_learn_mode_info.bSource = 0;
  my_zwapi_set_learn_mode_cb_save(&test_learn_mode_info);

  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);
  zwave_controller_on_error_Expect(
    ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_LEARN_MODE_FAIL);
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

// Test aborting learn mode
void test_zwave_network_management_learn_mode_abort()
{
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_NWI, NULL, SL_STATUS_OK);
  zwapi_set_learn_mode_IgnoreArg_completedFunc();
  zwapi_set_learn_mode_AddCallback(my_zwapi_set_learn_mode_stub);
  zwave_controller_on_state_updated_Ignore();
  zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());

  zwave_network_management_abort();
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());

  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_add_node_false_positive_inclusion()
{
  /* Check if the Network management state is idle */
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // First it'll attempt to stop all network operations
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  // Mocking the zwapi_add_node_from_network function call
  zwapi_add_node_to_network_ExpectAndReturn(
    (ADD_NODE_ANY | ADD_NODE_OPTION_NETWORK_WIDE),
    0,
    SL_STATUS_OK);
  // Ignoring the completedFunc argument of zwapi_add_node_from_network
  zwapi_add_node_to_network_IgnoreArg_completedFunc();
  zwave_controller_on_state_updated_Ignore();
  zwave_nodemask_t node_list;
  memset(node_list, 0, sizeof(zwave_nodemask_t));
  node_list[0] = 0b00101101;  // 4 nodes in the network, NodeID 1, 3, 4 and 6
  zwapi_get_full_node_list_StopIgnore();
  zwapi_get_full_node_list_ExpectAndReturn(0, SL_STATUS_OK);
  zwapi_get_full_node_list_IgnoreArg_node_list();
  zwapi_get_full_node_list_ReturnMemThruPtr_node_list(node_list,
                                                      sizeof(zwave_nodemask_t));

  zwapi_add_node_to_network_AddCallback(my_s2_add_node_stub);

  zwave_network_management_add_node();
  contiki_test_helper_run(0);

  /* When we return here the state of NM should be NM_WAITING_FOR_ADD */
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  node_id = 0x04;  // This NodeID is already in our network
  LEARN_INFO sample_add_node_report = {ADD_NODE_STATUS_NODE_FOUND, node_id};
  sample_add_node_report.bStatus    = ADD_NODE_STATUS_NODE_FOUND;
  sample_add_node_report.bSource    = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NODE_FOUND, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_ADDING_END_NODE;
  sample_add_node_report.pCmd    = node_add_nif_with_s2;
  sample_add_node_report.bSource = node_id;
  sample_add_node_report.bLen    = sizeof(node_add_nif_with_s2);
  my_s2_add_node_callback_save(&sample_add_node_report);

  sample_nif->basic_device_class    = BASIC_TYPE_END_NODE;
  sample_nif->command_class_list[0] = COMMAND_CLASS_ZWAVEPLUS_INFO;
  sample_nif->command_class_list[1] = COMMAND_CLASS_SECURITY_2;

  zwave_command_class_list_unpack_Expect(NULL,
                                         &sample_add_node_report.pCmd[3],
                                         sample_add_node_report.bLen - 3);
  zwave_command_class_list_unpack_ReturnThruPtr_node_info(sample_nif);
  zwave_command_class_list_unpack_IgnoreArg_node_info();

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_PROTOCOL, zwave_network_management_get_state());

  sample_add_node_report.bStatus = ADD_NODE_STATUS_DONE;
  sample_add_node_report.bSource = node_id;
  my_s2_add_node_callback_save(&sample_add_node_report);

  zwapi_add_node_to_network_ExpectAndReturn((ADD_NODE_STOP), 0, SL_STATUS_OK);
  zwapi_add_node_to_network_IgnoreArg_completedFunc();

  contiki_test_helper_run(0);

  //NM should be back in NM_IDLE state, the node was already part of the network
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Restart ignoring zwapi_get_node_list
  zwapi_get_full_node_list_IgnoreAndReturn(SL_STATUS_OK);
}

void test_zwave_network_management_request_node_neighbor_discovery()
{
  zwave_node_id_t test_node_id = 0x55;
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  zwapi_request_neighbor_update_Stub(my_zwapi_request_neighbor_update_stub);
  zwave_network_management_request_node_neighbor_discovery(test_node_id);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NEIGHBOR_DISCOVERY_ONGOING,
                    zwave_network_management_get_state());

  uint8_t neighbor_update_status = 0x22;
  zwave_controller_on_request_neighbor_update_Expect(neighbor_update_status);
  request_neighbor_update_callback_save(neighbor_update_status);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(10);
  TEST_ASSERT_EQUAL(request_neighbor_node_id, test_node_id);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_abort_request_node_neighbor_discovery()
{
  zwave_node_id_t test_node_id = 0x55;
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  zwapi_request_neighbor_update_Stub(my_zwapi_request_neighbor_update_stub);
  zwave_network_management_request_node_neighbor_discovery(test_node_id);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NEIGHBOR_DISCOVERY_ONGOING,
                    zwave_network_management_get_state());

  zwave_controller_on_request_neighbor_update_Expect(0x23);
  zwave_network_management_abort();
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(10);
  TEST_ASSERT_EQUAL(request_neighbor_node_id, test_node_id);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_node_neighbor_discovery_rejected()
{
  zwave_node_id_t test_node_id = 300;
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  zwave_network_management_request_node_neighbor_discovery(test_node_id);
  zwave_controller_on_state_updated_Ignore();
  zwapi_request_neighbor_update_ExpectAndReturn(test_node_id,
                                                NULL,
                                                SL_STATUS_FAIL);
  zwapi_request_neighbor_update_IgnoreArg_completedFunc();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_start_proxy_inclusion()
{
  zwave_node_id_t my_test_node_id = 0x19;
  // setting the test node information
  sample_nif->basic_device_class        = BASIC_TYPE_END_NODE;
  sample_nif->generic_device_class      = GENERIC_TYPE_SENSOR_NOTIFICATION;
  sample_nif->specific_device_class     = SPECIFIC_TYPE_NOTIFICATION_SENSOR;
  sample_nif->command_class_list_length = 2;
  sample_nif->command_class_list[0]     = COMMAND_CLASS_ZWAVEPLUS_INFO;
  sample_nif->command_class_list[1]     = COMMAND_CLASS_SECURITY_2;
  sample_nif->command_class_list[3]     = COMMAND_CLASS_SECURITY;
  sample_nif->listening_protocol
    = ZWAVE_NODE_INFO_LISTENING_PROTOCOL_ROUTING_MASK;
  sample_nif->optional_protocol
    = ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_END_NODE_MASK;

  zwapi_get_protocol_info_ExpectAndReturn(my_test_node_id, 0, SL_STATUS_OK);
  zwapi_get_protocol_info_IgnoreArg_node_info_header();
  zwapi_node_info_header_t nodeInfo;
  nodeInfo.capability         = ZWAVE_NODE_INFO_LISTENING_PROTOCOL_ROUTING_MASK;
  nodeInfo.security           = ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_END_NODE_MASK;
  nodeInfo.reserved           = 0x44;
  nodeInfo.node_type.basic    = sample_nif->basic_device_class;
  nodeInfo.node_type.generic  = sample_nif->generic_device_class;
  nodeInfo.node_type.specific = sample_nif->specific_device_class;
  zwapi_get_protocol_info_ReturnThruPtr_node_info_header(&nodeInfo);
  zwave_s2_start_add_node_Expect(my_test_node_id);
  zwave_network_management_start_proxy_inclusion(my_test_node_id,
                                                 *sample_nif,
                                                 INITIATE_PROXY_INCLUSION);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_ADD,
                    zwave_network_management_get_state());

  zwave_controller_on_node_added_ExpectWithArray(
    SL_STATUS_OK,
    (zwave_node_info_t *)sample_nif,
    sizeof(nif_buffer),
    my_test_node_id,
    dsk,
    sizeof(dsk),
    0x87,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,
    PROTOCOL_ZWAVE);
  zwave_controller_on_node_added_IgnoreArg_nif();
  zwave_controller_on_node_added_IgnoreArg_dsk();
  my_sec2_inclusion_cb.on_inclusion_complete(
    granted_keys,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_inclusion_protocol_is_saved()
{
  inclusion_protocol = PROTOCOL_ZWAVE_LONG_RANGE;
  smart_start_common();
  zwave_s2_dsk_accept_Expect(1, dsk, 2);
  zwave_dsk_t reported_key;
  memcpy(reported_key, dsk, sizeof(zwave_dsk_t));
  reported_key[0] = 0;
  reported_key[1] = 0;

  my_sec2_inclusion_cb.on_dsk_challenge(granted_keys, 2, reported_key);
  contiki_test_helper_run(0);

  zwave_controller_on_node_added_ExpectWithArray(
    SL_STATUS_OK,
    (zwave_node_info_t *)sample_nif,
    sizeof(nif_buffer),
    node_id,
    dsk,
    sizeof(dsk),
    granted_keys,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,
    PROTOCOL_ZWAVE_LONG_RANGE);

  expect_smart_start_enable();
  my_sec2_inclusion_cb.on_inclusion_complete(
    granted_keys,
    ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
  zwave_controller_on_state_updated_Ignore();
  contiki_test_helper_run(0);
}

void test_zwave_network_management_assign_return_route()
{
  zwave_node_id_t test_zpc_node_id = 0x07;
  zwave_node_id_t test_node_id     = 0x08;
  zwapi_assign_return_route_AddCallback(zwapi_assign_return_route_stub);
  zwave_controller_on_state_updated_Ignore();
  zwapi_assign_return_route_ExpectAndReturn(test_node_id,
                                            test_zpc_node_id,
                                            NULL,
                                            SL_STATUS_OK);
  zwapi_assign_return_route_IgnoreArg_completedFunc();
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  zwave_network_management_assign_return_route(test_node_id, test_zpc_node_id);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_ASSIGNING_RETURN_ROUTE,
                    zwave_network_management_get_state());

  zwapi_assign_return_callback_save(0x00);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_assign_return_route_back_to_back()
{
  zwave_node_id_t test_zpc_node_id = 0x01;
  zwave_node_id_t test_node_id     = 0x02;
  zwave_node_id_t test_node_id_2   = 0x03;
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  zwapi_assign_return_route_AddCallback(zwapi_assign_return_route_stub);

  zwave_controller_on_state_updated_Ignore();
  zwapi_assign_return_route_ExpectAndReturn(test_node_id,
                                            test_zpc_node_id,
                                            NULL,
                                            SL_STATUS_OK);
  zwapi_assign_return_route_IgnoreArg_completedFunc();
  zwave_network_management_assign_return_route(test_node_id, test_zpc_node_id);
  zwave_network_management_assign_return_route(test_node_id_2,
                                               test_zpc_node_id);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_ASSIGNING_RETURN_ROUTE,
                    zwave_network_management_get_state());
  zwapi_assign_return_route_ExpectAndReturn(test_node_id_2,
                                            test_zpc_node_id,
                                            NULL,
                                            SL_STATUS_OK);
  zwapi_assign_return_route_IgnoreArg_completedFunc();
  zwapi_assign_return_callback_save(0x00);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_ASSIGNING_RETURN_ROUTE,
                    zwave_network_management_get_state());
}

void test_idle_events()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  my_sec2_inclusion_cb.on_dsk_challenge(granted_keys, 2, 0);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  my_sec2_inclusion_cb.on_keys_request(0, 0);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  my_sec2_inclusion_cb.on_inclusion_complete(0, 0);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_activate_new_operations_while_controller_reset()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Get the Z-Wave Controller to be resetting.
  zwave_controller_is_reset_ongoing_StopIgnore();

  // Activate NWE learn mode:
  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWE));

  // Activate NWI learn mode:
  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI));

  // Activate learn mode of the future:
  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_network_management_learn_mode(
                      ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START + 3));

  // Try to remove failed NodeID 2
  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwave_network_management_remove_failed(2));

  // Activate Remove node
  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwave_network_management_remove_node());

  // Activate Add node with DSK
  const zwave_dsk_t dsk = {};
  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_network_management_add_node_with_dsk(dsk, 0, 0));

  // Activate Add node.
  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwave_network_management_add_node());

  // Start a neighbor discovery during reset:
  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    zwave_network_management_request_node_neighbor_discovery(2));

  // Start a neighbor discovery during reset:
  zwave_node_info_t nif = {};
  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_network_management_start_proxy_inclusion(3, nif, 4));

  // Try to assign return routes
  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_network_management_assign_return_route(3, 4));
}
