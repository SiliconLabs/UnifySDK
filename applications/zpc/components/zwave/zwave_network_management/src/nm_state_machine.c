/**
 * @file nm_state_machine.c
 * @brief state machine for zwave_network_management
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
#include <stdlib.h>

#include "state_logging.h"
#include "nm_state_machine.h"
#include "zwave_network_management.h"
#include "zwave_network_management_types.h"
#include "zwave_rx.h"

#include "zwave_network_management_process.h"
#include "zwave_network_management_helpers.h"
#include "zwave_network_management_callbacks.h"
#include "zwave_network_management_return_route_queue.h"

// Interfaces
#include "zwave_helper_macros.h"
#include "ZW_classcmd.h"

// Z-Wave/ZPC Components
#include "zwapi_protocol_controller.h"
#include "zwapi_protocol_mem.h"
#include "zwapi_init.h"
#include "zwave_controller_internal.h"
#include "zwave_controller_types.h"
#include "zwave_controller_utils.h"
#include "zwave_controller_storage.h"
#include "zwave_s0_network.h"
#include "zwave_s2_network.h"
#include "zwave_s2_keystore.h"
#include "zwave_tx.h"
#include "zwave_smartstart_management.h"

// Unify components
#include "sl_log.h"
#include "process.h"
#include "sys/etimer.h"

#define LOG_TAG "nm_state_machine"

#define ADD_NODE_HOME_ID     8
#define ADD_NODE_STOP        5
#define ADD_NODE_STOP_FAILED 6
#define OBFUSCATED_DSK_LEN   2
#define ACCEPT_DSK           1
#define REJECT_DSK           0

network_mgmt_state_t nms;
static LEARN_INFO inf_bkup;
static uint8_t info_buf[NODE_INFO_MAX_SIZE];
PROCESS_NAME(zwave_network_management_process);

/* Ctimer used to abort s2 incl asynchronously to avoid a buffer overwrite
 * in libs2. */

static uint8_t get_event_for_learn_status(uint8_t learn_info_status)
{
  switch (learn_info_status) {
    case ADD_NODE_STATUS_LEARN_READY:
      return NM_EV_ADD_LEARN_READY;
    case ADD_NODE_STATUS_NODE_FOUND:
      return NM_EV_ADD_NODE_FOUND;
    case ADD_NODE_STATUS_ADDING_CONTROLLER:
      return NM_EV_ADD_CONTROLLER;
    case ADD_NODE_STATUS_PROTOCOL_DONE:
      return NM_EV_ADD_PROTOCOL_DONE;
    case ADD_NODE_STATUS_ADDING_END_NODE:
      return NM_EV_ADD_END_NODE;
    case ADD_NODE_STATUS_FAILED:
      return NM_EV_ADD_FAILED;
    case ADD_NODE_STATUS_NOT_PRIMARY:
      return NM_EV_ADD_NOT_PRIMARY;
    case ADD_NODE_STATUS_DONE:
      return NM_EV_ADD_NODE_STATUS_DONE;
    default:
      sl_log_error(LOG_TAG, "unknown status:%d\n", learn_info_status);
  }

  return 0;
}

/**
 * Callback for zwapi_add_node_to_network()
 */
static void add_node_status_update(LEARN_INFO *inf)
{
  memset(&inf_bkup, 0, sizeof(inf_bkup));

  if (inf->bLen > NODE_INFO_MAX_SIZE) {
    sl_log_error(LOG_TAG,
                 "Dropping status update %d for NodeID: %d. max buffer size"
                 "exceeded by %d bytes.\n",
                 inf->bStatus,
                 inf->bSource,
                 inf->bLen - NODE_INFO_MAX_SIZE);
  } else {
    inf_bkup.pCmd    = info_buf;
    inf_bkup.bStatus = inf->bStatus;
    inf_bkup.bSource = inf->bSource;
    inf_bkup.bLen    = inf->bLen;
    memcpy(inf_bkup.pCmd, inf->pCmd, inf->bLen);
  }

  const uint8_t ev = get_event_for_learn_status(inf->bStatus);
  process_post(&zwave_network_management_process, ev, &inf_bkup);
}

static void on_secure_inclusion_complete(zwave_keyset_t granted_keys,
                                         zwave_kex_fail_type_t kex_fail_code)
{
  if ((nms.state == NM_WAIT_FOR_SECURE_ADD)
      || (nms.state == NM_WAIT_FOR_SECURE_LEARN)) {
    nms.granted_keys  = granted_keys;
    nms.kex_fail_type = kex_fail_code;
    process_post(&zwave_network_management_process, NM_EV_SECURITY_DONE, 0);
  }
}

static void on_keys_request(zwave_keyset_t requested_keys, bool csa)
{
  if (nms.state != NM_WAIT_FOR_SECURE_ADD)
    return;
  nms.requested_csa  = csa;
  nms.requested_keys = requested_keys;
  process_post(&zwave_network_management_process,
               NM_EV_ADD_SECURITY_REQ_KEYS,
               0);
}

static void
  on_dsk_challenge(zwave_keyset_t granted_keys, int dsk_length, zwave_dsk_t dsk)
{
  if ((nms.state != NM_WAIT_FOR_SECURE_ADD)
      && (nms.state != NM_WAIT_FOR_SECURE_LEARN))
    return;
  nms.granted_keys = granted_keys;
  memcpy(nms.reported_dsk, dsk, sizeof(zwave_dsk_t));
  nms.reported_dsk_blanked = dsk_length;
  process_post(&zwave_network_management_process,
               NM_EV_ADD_SECURITY_KEY_CHALLENGE,
               0);
}

static void on_s2_inclusion_started()
{
  // Mark that we initiated an S2 bootstrapping (so we know the difference
  // between no granted keys via bootstrapping or just a timeout)
  nms.s2_bootstrapping_started = true;
  zwave_s0_stop_bootstrapping();
}

/**
 * Callback for Remove a failed / self-destructed node
 * Notify the FSM of status of RemoveFailed
 *
 * \param status One of the zwapi_remove_failed_node() callback statuses:
 *        ZW_NODE_OK
 *        ZW_FAILED_NODE_REMOVED
 *        ZW_FAILED_NODE_NOT_REMOVED.
 */

static void on_remove_failed_status_update(uint8_t status)
{
  nm_event_t ev = NM_EV_REMOVE_FAILED_FAIL;
  if ((status == ZW_NODE_OK) || (status == ZW_FAILED_NODE_NOT_REMOVED)
      || (status == NM_WAITING_FOR_FAILED_NODE_REMOVAL)) {
    ev = NM_EV_REMOVE_FAILED_FAIL;
  } else if (status == ZW_FAILED_NODE_REMOVED) {
    ev = NM_EV_REMOVE_FAILED_OK;
  }
  process_post(&zwave_network_management_process, ev, 0);
}

static void on_remove_node_status_update(LEARN_INFO *remove_node_information)
{
  switch (remove_node_information->bStatus) {
    case REMOVE_NODE_STATUS_LEARN_READY:
      sl_log_debug(LOG_TAG,
                   "ZPC is now ready to remove nodes, listening for NIFs\n");
      break;
    case REMOVE_NODE_STATUS_NODE_FOUND:
      sl_log_debug(
        LOG_TAG,
        "Node found, the Z-Wave Protocol is initiating network exclusion...\n");
      break;

    case REMOVE_NODE_STATUS_REMOVING_END_NODE:
    case REMOVE_NODE_STATUS_REMOVING_CONTROLLER:
      // At that point, we get notified of which NodeID is being excluded.
      // Save it somewhere as when it is not told in any of the other callbacks
      nms.node_id_being_handled = remove_node_information->bSource;
      sl_log_debug(LOG_TAG,
                   "ZPC is now removing node (NodeID "
                   "%d) from its network\n",
                   remove_node_information->bSource);
      break;

    case REMOVE_NODE_STATUS_DONE:
      sl_log_debug(LOG_TAG,
                   "Remove Node operation completed for NodeID %d\n",
                   nms.node_id_being_handled);
      process_post(&zwave_network_management_process,
                   NM_EV_NODE_REMOVE_COMPLETE,
                   0);
      break;

    case REMOVE_NODE_STATUS_FAILED:
      process_post(&zwave_network_management_process,
                   NM_EV_NODE_REMOVE_FAILED,
                   0);
      break;

    case ADD_NODE_STATUS_NOT_PRIMARY:
      sl_log_debug(LOG_TAG,
                   "The requested operation cannot be performed since the ZPC "
                   "is not Primary Controller\n");
      process_post(&zwave_network_management_process,
                   NM_EV_NODE_REMOVE_FAILED,
                   0);
      break;
  }
}

/**
 * @brief Tell the Z-Wave Controller that a Network Inclusion operation
 * has completed and returns the NM state machine to IDLE.
 *
 */
static void dispatch_node_added()
{
  zwave_controller_on_node_added(SL_STATUS_OK,
                                 &nms.node_info,
                                 nms.node_id_being_handled,
                                 nms.reported_dsk,
                                 nms.granted_keys,
                                 nms.kex_fail_type,
                                 nms.inclusion_protocol);
  network_management_refresh_cached_node_list();
  nms.state                = NM_IDLE;
  nms.proxy_inclusion_step = 0;
}

/**
 * @brief This function is called at the end of a set default or after
 * learnmode.
 *
 * @param kex_fail S2 failure code if any
 */
static void on_new_network(zwave_kex_fail_type_t kex_fail)
{
  /* Refresh the cached network properties*/
  network_management_refresh_network_information();
  nms.granted_keys = zwave_s2_keystore_get_assigned_keys();

  // Take the SUC/SIS role if noboby has it in our new network.
  network_management_take_sis_role_if_no_suc_in_network();

  //Call the upper layer callbacks
  zwave_controller_on_new_network_entered(nms.cached_home_id,
                                          nms.cached_local_node_id,
                                          nms.granted_keys,
                                          kex_fail);
}

static void
  nop_send_done(uint8_t status, const zwapi_tx_report_t *tx_info, void *user)
{
  (void)tx_info;
  (void)user;
  sl_log_debug(LOG_TAG,
               "NOP transmission completed. "
               "Tx Status: %d (0 for Ok, 1 for No Ack, 2 for Fail)",
               status);
  if (false == IS_TRANSMISSION_SUCCESSFUL(status)) {
    process_post(&zwave_network_management_process, NM_EV_NOP_FAIL, 0);
  } else {
    process_post(&zwave_network_management_process, NM_EV_NOP_SUCCESS, 0);
  }
}

void nm_state_machine_teardown()
{
  memset(&nms, 0, sizeof(nms));
  network_management_stop_ongoing_operations();
}

void nm_state_machine_init()
{
  zwave_s2_network_callbacks_t cb = {
    .on_inclusion_complete = on_secure_inclusion_complete,
    .on_keys_request       = on_keys_request,
    .on_dsk_challenge      = on_dsk_challenge,
    .on_inclusion_started  = on_s2_inclusion_started,
  };
  zwave_s2_set_network_callbacks(&cb);
  zwave_s0_set_network_callbacks(on_secure_inclusion_complete);
  memset(&nms, 0, sizeof(nms));
  nms.s0_inclusion_enabled = true;
  nms.state = NM_IDLE;

  network_management_refresh_network_information();
  network_management_stop_ongoing_operations();
  zwave_network_management_return_route_clear_queue();

  // Print the HomeID, NodeID and DSK to console
  sl_log_info(LOG_TAG,
              "ZPC HomeID %08X - NodeID %d",
              nms.cached_home_id,
              nms.cached_local_node_id);
  zwave_dsk_t dsk;
  zwave_s2_keystore_get_dsk(ZWAVE_S2_KEYSTORE_STATIC_ECDH_KEY, dsk);
  zwave_sl_log_dsk(LOG_TAG, dsk);

  // Print our Node list.
  for (zwave_node_id_t node_id = ZW_MIN_NODE_ID; node_id <= ZW_LR_MAX_NODE_ID;
       node_id++) {
    if (!ZW_IS_NODE_IN_MASK(node_id, nms.cached_node_list)) {
      continue;
    }
    sl_log_info(LOG_TAG, "NodeID %d is present in our network", node_id);
  }

  network_management_take_sis_role_if_no_suc_in_network();
}

void nm_fsm_post_event(nm_event_t ev, void *event_data)
{
  sl_log_debug(LOG_TAG,
               "nm_fsm_post_event event: %s state: %s\n",
               nm_event_name(ev),
               nm_state_name(nms.state));

  switch (nms.state) {
    case NM_IDLE:
      if (ev == NM_EV_LEARN_SET) {
        // Make sure to stop ongoing network management operations
        network_management_stop_ongoing_operations();
        if (nms.learn_mode_intent == ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI) {
          zwapi_set_learn_mode(LEARN_MODE_NWI, &on_learn_mode_callback);
        } else if (nms.learn_mode_intent
                   == ZWAVE_NETWORK_MANAGEMENT_LEARN_NWE) {
          zwapi_set_learn_mode(LEARN_MODE_NWE, &on_learn_mode_callback);
        } else {
          sl_log_warning(LOG_TAG,
                         "Learn mode set with Neither NWI or NWE. Ignoring.");
          return;
        }
        nms.state = NM_LEARN_MODE;
        etimer_set(&nms.timer, LEARN_MODE_TIMEOUT);
      }
      if (ev == NM_EV_SET_DEFAULT) {
        nms.state = NM_SET_DEFAULT;
        zwapi_set_default(&on_set_default_complete);
        etimer_set(&nms.timer, SET_DEFAULT_TIMEOUT);
      } else if (ev == NM_EV_REMOVE_FAILED) {
        nms.state = NM_SEND_NOP;
        process_post(&zwave_network_management_process,
                     NM_EV_REMOVE_FAILED,
                     &inf_bkup);
      } else if (ev == NM_EV_SMART_START_ENABLE) {
        if (!nms.smart_start_add_mode_enabled) {
          // If SmartStart is disabled then we stop the SmartStart mode
          // If its enabled then we will enable this when this function exits.
          zwapi_add_node_to_network(ADD_NODE_STOP, NULL);
        }
      } else if (ev == NM_EV_NODE_ADD_SMART_START) {
        nms.inclusion_protocol     = PROTOCOL_ZWAVE;
        smartstart_event_data_t *e = (smartstart_event_data_t *)event_data;
        uint8_t inclusion_mode     = ADD_NODE_HOME_ID;
        if (e->preferred_inclusion == PROTOCOL_ZWAVE_LONG_RANGE) {
          inclusion_mode |= ADD_NODE_OPTION_LONG_RANGE;
          nms.inclusion_protocol = PROTOCOL_ZWAVE_LONG_RANGE;
        }

        sl_log_info(LOG_TAG,
                    "Attempting to SmartStart include node with NWI HomeID "
                    "%02X %02X %02X %02X using %s",
                    e->dsk[8] | 0xC0,  // Bits 7 and 6 at 1
                    e->dsk[9],
                    e->dsk[10],
                    e->dsk[11] & 0xFE,  // Bit 0 at 0
                    ((e->preferred_inclusion == PROTOCOL_ZWAVE_LONG_RANGE)
                       ? "Z-Wave Long Range"
                       : "Z-Wave"));
        zwapi_add_smartstart_node_to_network(inclusion_mode,
                                             (uint8_t *)&e->dsk[8],
                                             add_node_status_update);
        nms.state        = NM_WAITING_FOR_ADD;
        nms.granted_keys = e->granted_keys;
        nms.flags        = NMS_FLAG_S2_ADD | NMS_FLAG_SMART_START_INCLUSION;
        etimer_set(&nms.timer, ADD_REMOVE_TIMEOUT);

        /* We store the dsk, but we only consider it valid after S2
         * inclusion has succeeded. */
        memcpy(nms.expected_dsk, e->dsk, sizeof(zwave_dsk_t));

        /*TODO: we were sending COMMAND_SMART_START_JOIN_STARTED_REPORT here*/
      } else if (ev == NM_EV_NODE_REMOVE) {
        network_management_stop_ongoing_operations();
        nms.node_id_being_handled      = 0;
        sl_status_t remove_node_status = zwapi_remove_node_from_network(
          (REMOVE_NODE_ANY | REMOVE_NODE_OPTION_NETWORK_WIDE),
          on_remove_node_status_update);
        if (remove_node_status == SL_STATUS_OK) {
          nms.state = NM_WAITING_FOR_NODE_REMOVAL;
          etimer_set(&nms.timer, ADD_REMOVE_TIMEOUT);
        }
      } else if (ev == NM_EV_NODE_ADD) {
        network_management_stop_ongoing_operations();
        network_management_refresh_cached_node_list();
        zwapi_add_node_to_network(ADD_NODE_ANY | ADD_NODE_OPTION_NETWORK_WIDE,
                                  add_node_status_update);
        nms.state              = NM_WAITING_FOR_ADD;
        nms.flags              = NMS_FLAG_S2_ADD;
        nms.inclusion_protocol = PROTOCOL_ZWAVE;
        etimer_set(&nms.timer, ADD_REMOVE_TIMEOUT);
      } else if (ev == NM_EV_REQUEST_NODE_NEIGHBOR_REQUEST) {
        zwave_node_id_t node_id = (zwave_node_id_t)(intptr_t)event_data;
        if (SL_STATUS_OK
            == zwapi_request_neighbor_update(
              node_id,
              &on_request_node_neighbor_update_completed)) {
          nms.state                 = NM_NEIGHBOR_DISCOVERY_ONGOING;
          nms.node_id_being_handled = node_id;
        }
      } else if (ev == NM_EV_START_PROXY_INCLUSION) {
        nms.state = NM_WAIT_FOR_PROTOCOL;
        nm_fsm_post_event(NM_EV_ADD_NODE_STATUS_DONE, 0);
        return;
      } else if (ev == NM_EV_ASSIGN_RETURN_ROUTE_START) {
        if (SL_STATUS_IN_PROGRESS
            == zwave_network_management_return_route_assign_next()) {
          nms.state = NM_ASSIGNING_RETURN_ROUTE;
        }
      }
      break;
      // End of case NM_IDLE:
    case NM_SEND_NOP:
      if (ev == NM_EV_TIMEOUT) {
        sl_log_info(LOG_TAG,
                    "Initiating Self destruct verification for NodeID %d\n",
                    nms.node_id_being_handled);
        nms.state = NM_WAIT_FOR_TX_TO_SELF_DESTRUCT;
      } else if (ev == NM_EV_REMOVE_FAILED) {
        sl_log_debug(LOG_TAG,
                     "Sending a NOP to failed NodeID %d before removing it.\n",
                     nms.node_id_being_handled);
        nms.state = NM_FAILED_NODE_REMOVE;
      }
      uint8_t nop_frame[1] = {0};

      // Prepare the Connection Info data:
      zwave_controller_connection_info_t connection_info = {{}, {}, 0};
      connection_info.remote.node_id = nms.node_id_being_handled;
      connection_info.encapsulation  = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;

      // Prepare the Z-Wave TX options.
      zwave_tx_options_t tx_options = {0};
      tx_options.qos_priority       = ZWAVE_TX_QOS_MAX_PRIORITY;

      etimer_set(&nms.timer, NOP_FOR_REMOVE_FAILED_TIMEOUT);
      if (zwave_tx_send_data(&connection_info,
                             sizeof(nop_frame),
                             nop_frame,
                             &tx_options,
                             nop_send_done,
                             0,
                             0)) {
        nop_send_done(TRANSMIT_COMPLETE_FAIL, 0, NULL);
      }
      break;
    case NM_WAIT_FOR_TX_TO_SELF_DESTRUCT:
    case NM_FAILED_NODE_REMOVE:
      if (ev == NM_EV_NOP_FAIL) {
        if (nms.state == NM_WAIT_FOR_TX_TO_SELF_DESTRUCT) {
          nms.state = NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL;
          etimer_set(&nms.timer, SEND_DATA_TIMEOUT);
          sl_log_debug(LOG_TAG,
                       "Starting %d ms timer for Self destruct / Remove Offline"
                       "NodeID %u\n",
                       SEND_DATA_TIMEOUT,
                       nms.node_id_being_handled);
        } else if (nms.state == NM_FAILED_NODE_REMOVE) {
          nms.state = NM_WAITING_FOR_FAILED_NODE_REMOVAL;
        }
        if (zwapi_remove_failed_node(nms.node_id_being_handled,
                                     on_remove_failed_status_update)
            != ZW_FAILED_NODE_REMOVE_STARTED) {
          sl_log_error(LOG_TAG,
                       "Remove offline operation failed for NodeID: %d\n",
                       nms.node_id_being_handled);
          on_remove_failed_status_update(ZW_FAILED_NODE_NOT_REMOVED);
        }
      } else if (ev == NM_EV_TIMEOUT) {
        if (nms.state == NM_WAIT_FOR_TX_TO_SELF_DESTRUCT) {
          nms.kex_fail_type = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_UNKNOWN;
          dispatch_node_added();
        } else {
          sl_log_error(LOG_TAG,
                       "Timed out in getting any kind of callback in"
                       " response to sending NOP to NodeID: %d\n",
                       nms.node_id_being_handled);
          on_remove_failed_status_update(ZW_FAILED_NODE_NOT_REMOVED);
        }
        sl_log_error(LOG_TAG,
                     "Self Destruct/Remove Offline operation"
                     "failed\n");
        nms.state = NM_IDLE;
      } else if (ev == NM_EV_NOP_SUCCESS) {
        if (nms.state == NM_WAIT_FOR_TX_TO_SELF_DESTRUCT) {
          nms.kex_fail_type = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_UNKNOWN;
          dispatch_node_added();
        } else {
          on_remove_failed_status_update(ZW_FAILED_NODE_NOT_REMOVED);
        }
        sl_log_info(
          LOG_TAG,
          "NodeID: %d is reachable. Self Destruct/Remove Offline operation "
          "failed\n",
          nms.node_id_being_handled);
        nms.state = NM_IDLE;
      } else if (ev == NM_EV_ABORT) {
        // Abort the Remove Failed Operation
        sl_log_debug(LOG_TAG,
                     "Aborting Remove Offline operation for NodeID: %d\n",
                     nms.node_id_being_handled);
        on_remove_failed_status_update(ZW_FAILED_NODE_NOT_REMOVED);
        nms.state = NM_IDLE;
      }
      break;
    case NM_SET_DEFAULT:
      if (ev == NM_EV_SET_DEFAULT_COMPLETE) {
        etimer_stop(&nms.timer);
        // Tell the Z-Wave Controller that we are done resetting.
        zwave_controller_on_reset_step_complete(
          ZWAVE_CONTROLLER_ZWAVE_NETWORK_MANAGEMENT_RESET_STEP_PRIORITY);
        // Update the NMS data
        network_management_refresh_network_information();
        // Create new security keys
        // It's important to generate new keys before calling zwave_s2_network_init
        // and telling that we are in a new network here.
        zwave_s2_create_new_network_keys();
        zwave_s2_network_init();
        // Anounce that our NodeID / HomeID just changed.
        zwave_controller_on_network_address_update(nms.cached_home_id,
                                                   nms.cached_local_node_id);
        // We expect nothing more after entering our network, it is ready to be operated
        on_new_network(ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
        nms.state = NM_IDLE;
      } else if (ev == NM_EV_TIMEOUT) {
        sl_log_error(LOG_TAG,
                     "Timed out waiting for Z-Wave API callback "
                     "after set default. Attempting to resume operation");
        zwapi_soft_reset();
        zwave_rx_wait_for_zwave_api_to_be_ready();
        on_set_default_complete();
      }
      break;
    case NM_WAITING_FOR_ADD:
      if ((ev == NM_EV_ABORT) || (ev == NM_EV_TIMEOUT)
          || (ev == NM_EV_ADD_FAILED)) {
        nms.state = NM_IDLE;  //send_reply goto was doing this.
        zwapi_add_node_to_network(ADD_NODE_STOP, NULL);
        // If the add node is aborted via user, we should not send Network
        // Management error
        if (ev != NM_EV_ABORT) {
          zwave_controller_on_error(
            ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL);
        }
      } else if (ev == NM_EV_ADD_NODE_FOUND) {  // this event is from Serial api
                                                // from add_node_status_update
        nms.state = NM_NODE_FOUND;
        etimer_set(&nms.timer, ADD_NODE_STATUS_ADDING_END_NODE_TIMEOUT);
      }
      break;
    case NM_NODE_FOUND:
      // this event is from Serial api(i.e., zwave_api) from add_node_status_update
      if (ev == NM_EV_ADD_CONTROLLER || ev == NM_EV_ADD_END_NODE) {
        LEARN_INFO *inf = (LEARN_INFO *)event_data;

        if (inf->bLen && (inf->bSource != 0)) {
          // Start neighbor discovery timeout, waiting for the protocol
          etimer_set(&nms.timer, ADD_NODE_PROTOCOL_NEIGHBOR_DISCOVERY_TIMEOUT);
          nms.status                = inf->bStatus;
          nms.node_id_being_handled = inf->bSource;
          if (ev == NM_EV_ADD_CONTROLLER) {
            nms.is_controller = 1;
          } else if (ev == NM_EV_ADD_END_NODE) {
            nms.is_controller = 0;
          }

          //ZW->HOST: REQ | 0x4A | funcID | bStatus | bSource | bLen | basic |
          // generic | specific | command classes[ ]
          if ((inf->bLen) >= 3 && (inf->bLen < NODE_INFO_MAX_SIZE)) {
            nms.node_info.basic_device_class    = inf->pCmd[0];
            nms.node_info.generic_device_class  = inf->pCmd[1];
            nms.node_info.specific_device_class = inf->pCmd[2];
            zwave_command_class_list_unpack(&nms.node_info,
                                            &inf->pCmd[3],
                                            inf->bLen - 3);
          } else {
            sl_log_error(LOG_TAG, "node info len is invalid");
            nm_fsm_post_event(NM_EV_ADD_FAILED, 0);
            return;
          }
          nms.state = NM_WAIT_FOR_PROTOCOL;
        } else {
          nm_fsm_post_event(NM_EV_ADD_FAILED, 0);
          return;
        }
        // This is where we know the NodeID of new node...
        // Announce the new node only if it was really included
        if ((nms.node_id_being_handled != 0)
            && (ZW_IS_NODE_IN_MASK(nms.node_id_being_handled,
                                   nms.cached_node_list))
                 == false) {
          zwave_controller_on_node_id_assigned(nms.node_id_being_handled,
                                               nms.inclusion_protocol);
        }

      } else if (ev == NM_EV_ADD_FAILED || ev == NM_EV_TIMEOUT) {
        nms.state = NM_IDLE;  //send_reply goto was doing this.
        /* Add node failed - Application should indicate this to user */
        zwapi_add_node_to_network(ADD_NODE_STOP_FAILED, NULL);
        zwave_controller_on_error(ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL);
      }
      break;
    case NM_WAIT_FOR_PROTOCOL:
      if (ev == NM_EV_ADD_PROTOCOL_DONE) {
        zwapi_add_node_to_network(ADD_NODE_STOP, add_node_status_update);
      } else if (ev == NM_EV_ADD_NODE_STATUS_DONE) {
        zwapi_node_info_header_t ni;

        /* It is recommended to stop the process again here */
        zwapi_add_node_to_network(ADD_NODE_STOP, NULL);

        if ((nms.node_id_being_handled != 0)
            && (ZW_IS_NODE_IN_MASK(nms.node_id_being_handled,
                                   nms.cached_node_list)
                == true)
            && (nms.proxy_inclusion_step != INITIATE_PROXY_INCLUSION_REPLACE)) {
          sl_log_debug(
            LOG_TAG,
            "NodeID %d was already part of our network. Nothing changed.",
            nms.node_id_being_handled);
          // Do not proceed with security bootstrapping.
          nms.state = NM_IDLE;
          break;
        }

        /* Get the Capabilities and Security fields. */
        zwapi_get_protocol_info(nms.node_id_being_handled, &ni);
        nms.node_info.listening_protocol = ni.capability;
        nms.node_info.optional_protocol  = ni.security;

        /// If it is a SmartStart inclusion, we just start S2 bootstrapping
        if (nms.flags & NMS_FLAG_SMART_START_INCLUSION) {
          nms.state = NM_WAIT_FOR_SECURE_ADD;
          zwave_s2_start_add_node(nms.node_id_being_handled);
          zwave_controller_storage_as_set_node_s2_capable(
            nms.node_id_being_handled);
        } else if (is_command_class_in_nif(
                     nms.node_info.command_class_list,
                     nms.node_info.command_class_list_length,
                     COMMAND_CLASS_SECURITY_2)) {
          /// S2 inclusion starts, we set the flag to S2_ADD to ensure it is S2 inclusion
          nms.flags = NMS_FLAG_S2_ADD;
          nms.state = NM_WAIT_FOR_SECURE_ADD;
          zwave_s2_start_add_node(nms.node_id_being_handled);
          zwave_controller_storage_as_set_node_s2_capable(
            nms.node_id_being_handled);
        }
        else if (nms.s0_inclusion_enabled && is_command_class_in_nif(
                     nms.node_info.command_class_list,
                     nms.node_info.command_class_list_length,
                     COMMAND_CLASS_SECURITY)) {
          nms.state = NM_WAIT_FOR_SECURE_ADD;
          zwave_s0_start_bootstrapping(nms.node_id_being_handled,
                                       nms.is_controller);
        } else {
          // None secure inclusion is performed, therefore, we set the granted key to zero
          nms.granted_keys  = 0;
          nms.kex_fail_type = 0;
          dispatch_node_added();
        }
      } else if (ev == NM_EV_TIMEOUT || ev == NM_EV_ADD_FAILED
                 || ev == NM_EV_ADD_NOT_PRIMARY) {
        /* Add node failed - Application should indicate this to user */
        zwapi_add_node_to_network(ADD_NODE_STOP_FAILED, NULL);
        zwave_controller_on_error(ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL);
        nms.state = NM_SEND_NOP;
        etimer_set(&nms.timer, SMART_START_SELF_DESTRUCT_TIMEOUT);
      }
      break;
#if 0
    case NM_PREPARE_SUC_INCLISION:
      if(ev == NM_EV_TIMEOUT) {
        nms.state = NM_WIAT_FOR_SUC_INCLUSION;
        request_inclusion_controller_handover(
            nms.node_id_being_handled,
            (nms.cmd == FAILED_NODE_REPLACE),
            &inclusion_controller_complete );
      }
      break;
#endif
    case NM_WAIT_FOR_SECURE_ADD:
      if (ev == NM_EV_SECURITY_DONE) {
        if (nms.kex_fail_type == ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE) {
          dispatch_node_added();
        } else if (nms.flags & NMS_FLAG_SMART_START_INCLUSION) {
          nms.state = NM_SEND_NOP;
          etimer_set(&nms.timer, SMART_START_SELF_DESTRUCT_TIMEOUT);
          break;
        } else {
          dispatch_node_added();
        }
      } else if (ev == NM_EV_ADD_SECURITY_REQ_KEYS) {  //kex report arrived
        // We dont leave the NM_WAIT_FOR_SECURE_ADD here because
        // we are still just proxying for the Security FSM
        if (nms.flags & NMS_FLAG_SMART_START_INCLUSION) {
          zwave_s2_key_grant(0x01, nms.requested_keys & nms.granted_keys, 0);
        } else {
          zwave_controller_on_keys_report(nms.requested_csa,
                                          nms.requested_keys);
        }
      } else if (ev == NM_EV_ADD_SECURITY_KEY_CHALLENGE) {
        if (nms.flags & NMS_FLAG_SMART_START_INCLUSION) {
          // Check the rest of the dsk to see if it matches
          // what we expect.
          if (memcmp(&nms.reported_dsk[nms.reported_dsk_blanked],
                     &nms.expected_dsk[nms.reported_dsk_blanked],
                     sizeof(zwave_dsk_t) - nms.reported_dsk_blanked)
              == 0) {
            memcpy(nms.reported_dsk,
                   nms.expected_dsk,
                   nms.reported_dsk_blanked);
            zwave_s2_dsk_accept(ACCEPT_DSK,
                                nms.expected_dsk,
                                OBFUSCATED_DSK_LEN);
          } else {
            sl_log_debug(
              LOG_TAG,
              "SmartStart: Input DSK not found in provisioning list\n");
            sl_log_byte_arr(LOG_TAG,
                            SL_LOG_DEBUG,
                            nms.expected_dsk,
                            sizeof(zwave_dsk_t));
            sl_log_debug(LOG_TAG, "vs  \n");
            sl_log_byte_arr(LOG_TAG,
                            SL_LOG_DEBUG,
                            nms.reported_dsk,
                            sizeof(zwave_dsk_t));
            // Abort S2 inclusion since we do not have a matching PVL
            // entry after all
            zwave_s2_dsk_accept(REJECT_DSK, 0, 0);
          }
        } else {
          if (find_dsk_obfuscated_bytes_from_smart_start_list(
                nms.reported_dsk,
                OBFUSCATED_DSK_LEN)) {
            sl_log_debug(LOG_TAG,
                         "SmartStart: Input DSK found in provisioning list\n");
            sl_log_byte_arr(LOG_TAG,
                            SL_LOG_DEBUG,
                            nms.reported_dsk,
                            sizeof(zwave_dsk_t))
              zwave_s2_dsk_accept(ACCEPT_DSK,
                                  nms.reported_dsk,
                                  OBFUSCATED_DSK_LEN);
          } else {
            // Non-SmartStart inclusions sends the request upwards
            zwave_controller_on_dsk_report(nms.reported_dsk_blanked,
                                           nms.reported_dsk,
                                           nms.requested_keys);
          }
        }
      } else if (ev == NM_EV_ADD_SECURITY_KEYS_SET) {
        zwave_s2_key_grant(nms.accepted_s2_bootstrapping,
                           nms.requested_keys & nms.granted_keys,
                           nms.accepted_csa);
      } else if (ev == NM_EV_ADD_SECURITY_DSK_SET) {
        zwave_s2_dsk_accept(ACCEPT_DSK,
                            nms.verified_dsk_input,
                            OBFUSCATED_DSK_LEN);
        memcpy(nms.reported_dsk, nms.verified_dsk_input, OBFUSCATED_DSK_LEN);
      } else if (ev == NM_EV_ABORT
                 && !(nms.flags & NMS_FLAG_SMART_START_INCLUSION)) {
        // If Event is Abort and it isn't SmartStart inclusion, Reject the DSK
        zwave_s2_dsk_accept(REJECT_DSK, 0, 0);
      }
      break;
    case NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL:
    case NM_WAITING_FOR_FAILED_NODE_REMOVAL:
      if (ev == NM_EV_REMOVE_FAILED_OK) {
        sl_log_debug(LOG_TAG,
                     "Self Destruct/Remove Offline operation "
                     "successful!\n");
        network_management_refresh_cached_node_list();
        zwave_controller_on_node_deleted(nms.node_id_being_handled);
        nms.state = NM_IDLE;
      } else if (ev == NM_EV_REMOVE_FAILED_FAIL) {
        sl_log_error(LOG_TAG,
                     "Self Destruct/Removal Offline operation "
                     "failed\n");
        if (nms.state == NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL) {
          nms.kex_fail_type = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_UNKNOWN;
          dispatch_node_added();
        } else {
          zwave_controller_on_error(
            ZWAVE_NETWORK_MANAGEMENT_ERROR_FAILED_NODE_REMOVE_FAIL);
        }
        nms.state = NM_IDLE;
      } else if (ev == NM_EV_TIMEOUT) {
        /* Protocol should always call back, but if it did not, we reset as a
         * fall-back*/
        sl_log_debug(LOG_TAG,
                     "Timed out waiting for zwapi_remove_failed_node() of "
                     "self-destruct/remove-failed node\n");
        if (nms.state == NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL) {
          nms.kex_fail_type = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_UNKNOWN;
          dispatch_node_added();
        } else {
          zwave_controller_on_error(
            ZWAVE_NETWORK_MANAGEMENT_ERROR_FAILED_NODE_REMOVE_FAIL);
        }
        sl_log_error(LOG_TAG,
                     "Self Destruct/Remove Offline operation"
                     "failed\n");
        nms.state = NM_IDLE;
      }
      break;
    case NM_WAITING_FOR_NODE_REMOVAL:
      if (ev == NM_EV_TIMEOUT || ev == NM_EV_NODE_REMOVE_FAILED
          || ev == NM_EV_ABORT) {
        zwapi_remove_node_from_network(REMOVE_NODE_STOP, NULL);
        nms.state = NM_IDLE;
      } else if (ev == NM_EV_NODE_REMOVE_COMPLETE) {
        zwapi_remove_node_from_network(REMOVE_NODE_STOP, NULL);
        // Tell the controller (who then tells everybody) that a node was removed.
        network_management_refresh_cached_node_list();
        zwave_controller_on_node_deleted(nms.node_id_being_handled);
        nms.state = NM_IDLE;
      }
      break;

    case NM_LEARN_MODE:
      if (ev == NM_EV_TIMEOUT || ev == NM_EV_ABORT) {
        // Just stop trying when we time out or abort
        zwapi_set_learn_mode(LEARN_MODE_DISABLE, NULL);
        nms.state = NM_IDLE;
      } else if (ev == NM_EV_LEARN_STARTED) {
        /* Start security here to make sure the timers are started in time */
        // Do not accept S0 bootstrapping after a SmartStart inclusion.
        if (nms.learn_mode_intent == ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI) {
          zwave_s0_start_learn_mode(nms.node_id_being_handled);
          zwave_s2_start_learn_mode(nms.node_id_being_handled);
        } else if (nms.learn_mode_intent
                   == ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START) {
          zwave_s2_start_learn_mode(nms.node_id_being_handled);
        }

        /*Print the DSK just to be friendly */
        zwave_dsk_t dsk;
        zwave_s2_keystore_get_dsk(ZWAVE_S2_KEYSTORE_STATIC_ECDH_KEY, dsk);
        zwave_sl_log_dsk(LOG_TAG, dsk);
        nms.state = NM_LEARN_MODE_STARTED;
      } else if (ev == NM_EV_LEARN_DONE) {
        //This is a controller replication, ie we should not do anything.
        nms.state = NM_IDLE;
      }
      break;
    case NM_LEARN_MODE_STARTED:
      if (ev == NM_EV_LEARN_DONE) {
        zwave_home_id_t previous_home_id
          = zwave_network_management_get_home_id();
        zwave_node_id_t previous_node_id
          = zwave_network_management_get_node_id();

        network_management_refresh_cached_ids();
        network_management_refresh_cached_node_list();

        if (nms.learn_mode_intent == ZWAVE_NETWORK_MANAGEMENT_LEARN_NWE) {
          sl_log_info(LOG_TAG, "We were just excluded. Re-initializing\n");
          nms.state = NM_SET_DEFAULT;
          process_post(&zwave_network_management_process,
                       NM_EV_SET_DEFAULT_COMPLETE,
                       0);
        } else if ((previous_home_id == zwave_network_management_get_home_id())
                   || (previous_node_id
                       == zwave_network_management_get_node_id())) {
          zwave_s2_abort_join();
          zwave_s0_stop_bootstrapping();
          nms.state = NM_WAIT_FOR_SECURE_LEARN;
          process_post(&zwave_network_management_process,
                       NM_EV_SECURITY_DONE,
                       0);
        } else {
          // Get started with S2 bootstrapping, the non-secure part of the inclusion is done.
          zwave_controller_on_network_address_update(nms.cached_home_id,
                                                     nms.cached_local_node_id);
          zwave_s2_keystore_reset_assigned_keys();
          zwave_s2_neighbor_discovery_complete();
          nms.state = NM_WAIT_FOR_SECURE_LEARN;
        }
      } else if (ev == NM_EV_LEARN_FAILED) {
        zwapi_set_learn_mode(LEARN_MODE_DISABLE, NULL);
        zwave_controller_on_error(
          ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_LEARN_MODE_FAIL);
        nms.state = NM_IDLE;
      }
      break;
    case NM_WAIT_FOR_SECURE_LEARN:
      if (ev == NM_EV_ADD_SECURITY_KEY_CHALLENGE) {
        zwave_s2_dsk_accept(ACCEPT_DSK, 0, 0);

        //Now we assume that the sender knows S2
        zwave_controller_storage_as_set_node_s2_capable(
          nms.node_id_being_handled);
      } else if (ev == NM_EV_SECURITY_DONE) {
        if (we_should_self_destruct()) {
          sl_log_info(LOG_TAG,
                      "SmartStart inclusion (or its S2 bootstrapping) failed. "
                      "we will reset ourselves now.");
          // Self destruct ourselves if we could not get SmartStart'ed
          zwave_controller_on_error(
            ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_SMART_START_INCLUSION_SECURITY_FAIL);
          nms.state = NM_SET_DEFAULT;
          zwapi_set_default(&on_set_default_complete);
          return;
        }
        // Else happily accept our new network.
        on_new_network(nms.kex_fail_type);
        nms.state = NM_IDLE;
      } else if (ev == NM_EV_ABORT) {
        /* Aborting S2 will cause S2 to return a SECURITY_DONE with fail. */
        zwave_s2_abort_join();
      }
      break;
    case NM_ASSIGNING_RETURN_ROUTE:
      if (ev == NM_EV_ABORT) {
        sl_log_debug(LOG_TAG,
                     "Aborting the ongoing Assign Return Route operation.");
        // Somebody wants to abort assigning return routes, we clear the queue
        // and pretend that the last route has just completed
        zwave_network_management_return_route_clear_queue();
        on_assign_return_route_complete(0);
      } else if (ev == NM_EV_ASSIGN_RETURN_ROUTE_COMPLETED) {
        sl_log_debug(LOG_TAG, "Assign Return Route operation completed.");
        nms.state = NM_IDLE;
      }
      break;
    case NM_NEIGHBOR_DISCOVERY_ONGOING:
      if (ev == NM_EV_ABORT) {
        sl_log_debug(LOG_TAG,
                     "Forcing to move back to idle during Neighbor discovery.");
        on_request_node_neighbor_update_completed(0x23);
      } else if (ev == NM_EV_REQUEST_NODE_NEIGHBOR_REQUEST_COMPLETE) {
        nms.state = NM_IDLE;
      }
      break;
    case NM_REPLACE_FAILED_REQ:
    case NM_PREPARE_SUC_INCLISION:
    case NM_WAIT_FOR_SUC_INCLUSION:
    case NM_PROXY_INCLUSION_WAIT_NIF:
      break;
  }

  sl_log_debug(LOG_TAG,
               "Exit event: %s state: %s.\n",
               nm_event_name(ev),
               nm_state_name(nms.state));

  // If we are idle, ensure to clean up everything from the previous operations
  if (nms.state == NM_IDLE) {
    etimer_stop(&nms.timer);
    reset_nms_last_operation_data();
  }

  // Check if we want to assign some return routes:
  if (SL_STATUS_IN_PROGRESS
      == zwave_network_management_return_route_assign_next()) {
    nms.state = NM_ASSIGNING_RETURN_ROUTE;
  }

  // Tell the Z-Wave Controller which state we are currently in.
  zwave_controller_on_state_updated(nms.state);

  // Check if we want to activate SmartStart (Add or Learn mode)
  if (nms.state == NM_IDLE) {
    sl_log_debug(LOG_TAG,
                 "SmartStart Configuration: Add Mode %s, Learn Mode %s\n",
                 nms.smart_start_add_mode_enabled ? "enabled" : "disabled",
                 nms.smart_start_learn_mode_enabled ? "enabled" : "disabled");

    // Restart SmartStart on the Z-Wave module if idle and SmartStart is enabled.
    if (nms.smart_start_add_mode_enabled
        && zwave_network_management_is_zpc_sis()) {
      sl_log_debug(LOG_TAG, "Re-enabling SmartStart Add mode\n");
      zwapi_add_node_to_network(ADD_NODE_SMART_START
                                  | ADD_NODE_OPTION_NETWORK_WIDE,
                                NULL);
    } else if (nms.smart_start_learn_mode_enabled
               && we_are_alone_in_our_network()) {
      // If we are alone in our network, and no SmartStart Add Mode,
      // then try SmartStart learn mode.
      sl_log_debug(LOG_TAG, "Enabling SmartStart Learn mode\n");
      nms.learn_mode_intent = ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START;
      zwapi_set_learn_mode(LEARN_MODE_SMART_START, &on_learn_mode_callback);
    }
  }
}
