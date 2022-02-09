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

#include "nm_state_machine.h"
#include "state_logging.h"

#include "sl_log.h"
#include "zwapi_protocol_controller.h"
#include "zwapi_protocol_mem.h"
#include "zwapi_init.h"
#include "zwave_controller_internal.h"
#include "zwave_controller_types.h"
#include "zwave_controller_utils.h"
#include "zwave_controller_storage.h"
#include "zwave_network_management.h"
#include "zwave_network_management_state.h"
#include "zwave_s0_network.h"
#include "zwave_s2_network.h"
#include "zwave_s2_keystore.h"
#include "zpc_endian.h"
#include "process.h"
#include "sys/etimer.h"
#include "zwave_tx.h"
#include "ZW_classcmd.h"

#define LOG_TAG "nm_state_machine"

#define ADD_NODE_HOME_ID     8
#define ADD_NODE_STOP        5
#define ADD_NODE_STOP_FAILED 6

network_mgmt_state_t nms;
static LEARN_INFO inf_bkup;
static uint8_t info_buf[NODE_INFO_MAX_SIZE];
PROCESS_NAME(zwave_network_management_process);

//forward declarations
static void stop_ongoing_nm_operations();

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

static void on_inclusion_complete(zwave_keyset_t granted_keys,
                                  zwave_kex_fail_type_t kex_fail_code)
{
  if (nms.state != NM_WAIT_FOR_SECURE_ADD)
    return;
  nms.granted_keys  = granted_keys;
  nms.kex_fail_type = kex_fail_code;
  process_post(&zwave_network_management_process, NM_EV_SECURITY_DONE, 0);
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
  if (nms.state != NM_WAIT_FOR_SECURE_ADD)
    return;
  nms.granted_keys = granted_keys;
  memcpy(nms.reported_dsk, dsk, sizeof(zwave_dsk_t));
  nms.reported_dsk_blanked = dsk_length;
  process_post(&zwave_network_management_process,
               NM_EV_ADD_SECURITY_KEY_CHALLENGE,
               0);
}

static void on_inclusion_started()
{
  // TODO stop the S0 FSM
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

static void on_set_default_complete()
{
  process_post(&zwave_network_management_process,
               NM_EV_SET_DEFAULT_COMPLETE,
               0);
}

/// Checking if a given command class is listed in the NIF
static sl_status_t is_cc_in_nif(zwave_command_class_t *nif,
                                uint8_t nif_len,
                                zwave_command_class_t cc)
{
  int i;
  if (nif_len > ZWAVE_CONTROLLER_MAXIMUM_COMMAND_CLASS_LIST_LENGTH) {
    sl_log_error(
      LOG_TAG,
      "The NIF length is not correct, which means NIF is corrupted\n");
    return SL_STATUS_FAIL;
  } else {
    for (i = 0; i < nif_len; i++) {
      if (nif[i] == cc) {
        return SL_STATUS_OK;
      }
    }
  }

  return SL_STATUS_NOT_SUPPORTED;
}

/**
 * @brief a helper function that referesh nms cashed home_id.
 */
static void network_management_refresh_cached_ids()
{
  zwapi_memory_get_ids(&nms.cached_home_id, &nms.cached_local_node_id);
}

static void network_management_refresh_cached_node_list()
{
  zwapi_get_full_node_list(nms.cached_node_list);
}

static void dispatch_node_added(bool refresh_node_cache)
{
  zwave_controller_on_node_added(SL_STATUS_OK,
                                 &nms.node_info,
                                 nms.node_id_being_handled,
                                 nms.reported_dsk,
                                 nms.granted_keys,
                                 nms.kex_fail_type,
                                 nms.inclusion_protocol);
  if (refresh_node_cache) {
    network_management_refresh_cached_node_list();
  }
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
  zwave_home_id_t home_id;
  zwave_node_id_t node_id;
  zwave_keyset_t key_set;

  /* Set the home id and node id */
  zwapi_memory_get_ids(&home_id, &node_id);

  /* cache for upper layers */
  nms.cached_home_id       = home_id;
  nms.cached_local_node_id = node_id;

  uint8_t cap = zwapi_get_controller_capabilities();
  key_set     = zwave_s2_keystore_get_assigned_keys();

  //If there is not SUC in the network assign me the suc role.
  if ((cap & (CONTROLLER_NODEID_SERVER_PRESENT | CONTROLLER_IS_SECONDARY))
      == 0) {
    zwapi_set_suc_node_id(node_id, true, false, ZW_SUC_FUNC_NODEID_SERVER, 0);
  }

  // Refresh our cached list of nodes
  network_management_refresh_cached_node_list();

  //Call the upper layer callbacks
  zwave_controller_on_new_network_entered(home_id, node_id, key_set, kex_fail);
}

static void
  nop_send_done(uint8_t status, const zwapi_tx_report_t *tx_info, void *user)
{
  (void)tx_info;
  (void)user;
  sl_log_debug(LOG_TAG,
               "NOP transmission completed. Tx Status: %d (0 for Ok, 1 for No "
               "Ack, 2 for Fail)",
               status);
  if (status != TRANSMIT_COMPLETE_OK) {
    process_post(&zwave_network_management_process, NM_EV_NOP_FAIL, 0);
  } else {
    process_post(&zwave_network_management_process, NM_EV_NOP_SUCCESS, 0);
  }
}

/**
 * @brief callback function for @ref zwapi_set_learn_mode()
 */
static void on_learn_mode_callback(LEARN_INFO *learn_mode_info)
{
  switch (learn_mode_info->bStatus) {
    case LEARN_MODE_STARTED:
      sl_log_info(
        LOG_TAG,
        "New Learn Mode operation (inclusion/exclusion/replication) started\n");
      nms.node_id_being_handled = learn_mode_info->bSource;
      break;
    case LEARN_MODE_DONE:
      sl_log_info(LOG_TAG,
                  "Learn mode completed successfully. Back to idle.\n");
      zwave_home_id_t home_id_before_learn_mode_done
        = zwave_network_management_get_home_id();
      network_management_refresh_cached_ids();
      if (home_id_before_learn_mode_done != nms.cached_home_id) {
        if ((nms.learn_mode_intent == ZW_SET_LEARN_MODE_NWE)
            || (learn_mode_info->bSource == 0x01
                && nms.learn_mode_intent == ZW_SET_LEARN_MODE_DIRECT_RANGE)) {
          sl_log_info(LOG_TAG, "ZPC has been excluded from the network\n");
          nms.state = NM_SET_DEFAULT;
          process_post(&zwave_network_management_process,
                       NM_EV_SET_DEFAULT_COMPLETE,
                       0);
        } else if (nms.learn_mode_intent == ZW_SET_LEARN_MODE_NWI
                   || nms.learn_mode_intent == ZW_SET_LEARN_MODE_DIRECT_RANGE) {
          sl_log_info(LOG_TAG, "ZPC has joined a new network\n");
          on_new_network(ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
        }
      }
      nms.learn_mode_intent = ZW_SET_LEARN_MODE_DISABLE;
      process_post(&zwave_network_management_process, NM_EV_LEARN_STOP, 0);
      break;
    case LEARN_MODE_FAILED:
      sl_log_warning(LOG_TAG,
                     "Learn mode failed. No network changes. Back to idle.\n");
      nms.learn_mode_intent = ZW_SET_LEARN_MODE_DISABLE;
      process_post(&zwave_network_management_process, NM_EV_LEARN_STOP, 0);
      break;
    default:
      sl_log_warning(
        LOG_TAG,
        "Learn Mode Callback is called without accepted status value\n");
      break;
  }
}

/**
 * @brief a helper function that allows to stop ongoing network
 *        management operations.
 */
static void stop_ongoing_nm_operations()
{
  zwapi_add_node_to_network(ADD_NODE_STOP, NULL);
  zwapi_remove_node_from_network(REMOVE_NODE_STOP, NULL);
  zwapi_set_learn_mode(ZW_SET_LEARN_MODE_DISABLE, NULL);
}

/**
 * @brief a callback function for Request Node Neighbor Discovery Command
 */
static void on_request_node_neighbor_update(uint8_t status)
{
  zwave_controller_on_request_neighbor_update(status);
}

void nm_state_machine_teardown()
{
  memset(&nms, 0, sizeof(nms));
  nms.state = NM_IDLE;
  stop_ongoing_nm_operations();
}

void nm_state_machine_init()
{
  zwave_s2_network_callbacks_t cb = {
    .on_inclusion_complete = on_inclusion_complete,
    .on_keys_request       = on_keys_request,
    .on_dsk_challenge      = on_dsk_challenge,
    .on_inclusion_started  = on_inclusion_started,
  };
  zwave_s2_set_network_callbacks(&cb);
  zwave_s0_set_network_callbacks(on_inclusion_complete);
  memset(&nms, 0, sizeof(nms));
  nms.state = NM_IDLE;

  network_management_refresh_cached_ids();
  network_management_refresh_cached_node_list();
  stop_ongoing_nm_operations();

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
}

void nm_fsm_post_event(nm_event_t ev, void *event_data)
{
  sl_log_debug(LOG_TAG,
               "nm_fsm_post_event event: %s state: %s\n",
               nm_event_name(ev),
               nm_state_name(nms.state));

#if 0

  if((nms.state  ==NM_LEARN_MODE_STARTED) || (nms.state == NM_WAIT_FOR_SECURE_LEARN) ) {
    if(ev == NM_EV_S0_STARTED) {
      /* Make temporary NIF, used for inclusion */
      DBG_PRINTF("S0 inclusion has started. Setting net_scheme to S0\n");
      net_scheme = SECURITY_SCHEME_0;
      SetPreInclusionNIF(SECURITY_SCHEME_0);
      /*Make sure to clear the s2 keys.*/
      keystore_network_key_clear(KEY_CLASS_S2_UNAUTHENTICATED);
      keystore_network_key_clear(KEY_CLASS_S2_AUTHENTICATED);
      keystore_network_key_clear(KEY_CLASS_S2_ACCESS);
      /*Stop the S2 FSM */
      sec2_abort_join();
    }
  }
#endif
  switch (nms.state) {
    case NM_IDLE:
      if (ev == NM_EV_LEARN_SET) {
        if (nms.learn_mode_intent == ZW_SET_LEARN_MODE_DIRECT_RANGE) {
          nms.flags = 0;
        } else if (nms.learn_mode_intent == ZW_SET_LEARN_MODE_NWI) {
          nms.flags = NMS_FLAG_LEARNMODE_NWI;
        } else if (nms.learn_mode_intent == ZW_SET_LEARN_MODE_NWE) {
          nms.flags = NMS_FLAG_LEARNMODE_NWE;
        }
        // stop ongoing network management operations
        stop_ongoing_nm_operations();
        nms.state = NM_LEARN_MODE;
        nms.count = 0;
        zwapi_set_learn_mode(ZW_SET_LEARN_MODE_DIRECT_RANGE,
                             on_learn_mode_callback);
        if (nms.learn_mode_intent == ZW_SET_LEARN_MODE_DIRECT_RANGE) {
          etimer_set(&nms.timer, LEARN_MODE_TIMEOUT_DIRECT_RANGE);
        } else {
          etimer_set(&nms.timer, LEARN_MODE_TIMEOUT_NETWORK_WIDE);
        }
      }
      if (ev == NM_EV_SET_DEFAULT) {
        nms.state = NM_SET_DEFAULT;
        zwapi_set_default(on_set_default_complete);
      } else if (ev == NM_EV_REMOVE_FAILED) {
        nms.state = NM_SEND_NOP;
        process_post(&zwave_network_management_process,
                     NM_EV_REMOVE_FAILED,
                     &inf_bkup);
      } else if (ev == NM_EV_SMART_START_ENABLE) {
        if (!nms.smart_start_enabled) {
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
        nms.node_id_being_handled      = 0;
        sl_status_t remove_node_status = zwapi_remove_node_from_network(
          (REMOVE_NODE_ANY | REMOVE_NODE_OPTION_NETWORK_WIDE),
          on_remove_node_status_update);
        if (remove_node_status == SL_STATUS_OK) {
          nms.state = NM_WAITING_FOR_NODE_REMOVAL;
          etimer_set(&nms.timer, ADD_REMOVE_TIMEOUT);
        }
      } else if (ev == NM_EV_NODE_ADD) {
        network_management_refresh_cached_node_list();
        zwapi_add_node_to_network(ADD_NODE_ANY, add_node_status_update);
        nms.state              = NM_WAITING_FOR_ADD;
        nms.flags              = NMS_FLAG_S2_ADD;
        nms.inclusion_protocol = PROTOCOL_ZWAVE;
        etimer_set(&nms.timer, ADD_REMOVE_TIMEOUT);
      } else if (ev == NM_EV_REQUEST_NODE_NEIGHBOR_REQUEST) {
        zwave_node_id_t node_id = (zwave_node_id_t)(intptr_t)event_data;
        zwapi_request_neighbor_update(node_id, on_request_node_neighbor_update);
      } else if (ev == NM_EV_START_PROXY_INCLUSION) {
        nms.state = NM_WAIT_FOR_PROTOCOL;
        nm_fsm_post_event(NM_EV_ADD_NODE_STATUS_DONE, 0);
        return;
      }
#if 0
       else if (ev == NM_EV_REPLACE_FAILED_START || ev == NM_EV_REPLACE_FAILED_START_S2 || ev == NM_EV_REPLACE_FAILED_STOP)
//      {
//         ZW_FAILED_NODE_REPLACE_FRAME* f = (ZW_FAILED_NODE_REPLACE_FRAME*) event_data;
//         ZW_FAILED_NODE_REPLACE_STATUS_FRAME_EX* reply = (ZW_FAILED_NODE_REPLACE_STATUS_FRAME_EX*) &nms.buf;
//         nms.cmd = FAILED_NODE_REPLACE;
//         nms.node_id_being_handled = f->nodeId;
//         nms.state = NM_REPLACE_FAILED_REQ;
//
//         reply->cmdClass = COMMAND_CLASS_NETWORK_MANAGEMENT_INCLUSION;
//         reply->cmd = FAILED_NODE_REPLACE_STATUS;
//         reply->seqNo = nms.seq;
//         reply->nodeId = nms.node_id_being_handled;
//         reply->status = ZW_FAILED_NODE_REPLACE_FAILED;
//         reply->kexFailType=0x00;
//         reply->grantedKeys =0x00;
//         nms.buf_len = sizeof(ZW_FAILED_NODE_REPLACE_STATUS_FRAME_EX);
//
//         if(ev == NM_EV_REPLACE_FAILED_START_S2) {
//           nms.flags = NMS_FLAG_S2_ADD;
//         }
//         if(ev == NM_EV_REPLACE_FAILED_STOP) {
//           goto send_reply;
//         }
//
//         ZW_AddNodeToNetwork(ADD_NODE_STOP, NULL);
//
//         DBG_PRINTF("Replace failed, node %i \n",f->nodeId);
//         if (ZW_ReplaceFailedNode(f->nodeId, f->tx_options != TRANSMIT_OPTION_LOW_POWER,
//             ReplaceFailedNodeStatus) == ZW_FAILED_NODE_REMOVE_STARTED)
//         {
//           ctimer_set(&nms.timer, ADD_REMOVE_TIMEOUT * 10, timeout, 0);
//         } else {
//           WRN_PRINTF("replace failed not started\n");
//           goto send_reply;
//         }
//     }  else if( ev == NM_EV_START_PROXY_INCLUSION || ev == NM_EV_START_PROXY_REPLACE) {
//        nms.node_id_being_handled = *((uint8_t*)event_data);
//        nms.cmd = (ev == NM_EV_START_PROXY_INCLUSION) ? NODE_ADD : FAILED_NODE_REPLACE;
//
//        ZW_RequestNodeInfo(nms.node_id_being_handled,0);
//        nms.state = NM_PROXY_INCLUSION_WAIT_NIF;
//        ctimer_set(&nms.timer,5000,timeout,0);
//
//        /* Send inclusion request to unsolicited destination */
//        set_unsolicited_as_nm_dest();
//      }
#endif
      break;
#if 0
    case NM_REPLACE_FAILED_REQ:
      if(ev ==NM_EV_TIMEOUT || ev == NM_EV_REPLACE_FAILED_STOP || ev == NM_EV_REPLACE_FAILED_FAIL) {
        ZW_AddNodeToNetwork(ADD_NODE_STOP,0);
        goto send_reply;
      } if(ev == NM_EV_REPLACE_FAILED_DONE) {
        int common_flags = 0;
        nms.state = NM_WAIT_FOR_SECURE_ADD;

        /*Cache security flags*/
        if(nms.flags & NMS_FLAG_PROXY_INCLUSION){
          LEARN_INFO *inf = (LEARN_INFO *) event_data;
          if(NULL != inf)
          {
            uint8_t* nif = inf->pCmd+3;
            if(is_cc_in_nif(nif,inf->bLen-3,COMMAND_CLASS_SECURITY)) common_flags |=NODE_FLAG_SECURITY0;
            if(is_cc_in_nif(nif,inf->bLen-3,COMMAND_CLASS_SECURITY_2)) common_flags |=(NODE_FLAG_SECURITY2_ACCESS |NODE_FLAG_SECURITY2_UNAUTHENTICATED | NODE_FLAG_SECURITY2_AUTHENTICATED);
          }
        } else {
          common_flags = GetCacheEntryFlag(nms.node_id_being_handled);
        }

        ApplicationControllerUpdate(UPDATE_STATE_DELETE_DONE, nms.node_id_being_handled, 0, 0, NULL);

        rd_probe_lock(TRUE);

        uint8_t suc_node = ZW_GetSUCNodeID();

        if(suc_node != MyNodeID &&  SupportsCmdClass(suc_node, COMMAND_CLASS_INCLUSION_CONTROLLER)) {
           rNM_EV_ADD_NODE_FOUNDd_register_new_node(nms.node_id_being_handled, RD_NODE_FLAG_JUST_ADDED);
           ctimer_set(&nms.timer,CLOCK_SECOND*2,timeout,0);
           nms.state = NM_PREPARE_SUC_INCLISION;
           return;
        }

        rd_register_new_node(nms.node_id_being_handled, RD_NODE_FLAG_JUST_ADDED | RD_NODE_FLAG_ADDED_BY_ME);

        /* This is to keep the probe from doing a secure commands supported get */
        SetCacheEntryFlagMasked(nms.node_id_being_handled,
        NODE_FLAG_INFO_ONLY | NODE_FLAG_SECURITY0 | NODE_FLAG_KNOWN_BAD,
        NODE_FLAG_INFO_ONLY | NODE_FLAG_SECURITY0 | NODE_FLAG_KNOWN_BAD);
        rd_probe_lock(FALSE);

        if( (nms.flags & NMS_FLAG_S2_ADD) &&
            (common_flags & (NODE_FLAG_SECURITY2_ACCESS |NODE_FLAG_SECURITY2_UNAUTHENTICATED | NODE_FLAG_SECURITY2_AUTHENTICATED)))
        {
          sec2_start_add_node(nms.node_id_being_handled, SecureInclusionDone);
          return;
        }


        if(common_flags & NODE_FLAG_SECURITY0) {
          if(nms.flags & NMS_FLAG_PROXY_INCLUSION ) {
            inclusion_controller_you_do_it(SecureInclusionDone);
            return;
          } else if (!(nms.flags & NMS_FLAG_SMART_START_INCLUSION)) { /* SmartStart inclusions must never be downgraded to S0 */
            security_add_begin(nms.node_id_being_handled, nms.tx_options,
                isNodeController(nms.node_id_being_handled), SecureInclusionDone);
            return;
          }
        }

        /*This is a non secure node or the node has already been included securely*/
        ZW_FAILED_NODE_REPLACE_STATUS_FRAME_EX* reply = (ZW_FAILED_NODE_REPLACE_STATUS_FRAME_EX*) &nms.buf;
        reply->status = ZW_FAILED_NODE_REPLACE_DONE;

        nm_fsm_post_event(NM_EV_SECURITY_DONE, &zero);
      }
      break;
#endif
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
                       "Starting %d ms timer for Self destruct / Remove Failed "
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
                       "Failed node remove failed for NodeID: %d\n",
                       nms.node_id_being_handled);
          on_remove_failed_status_update(ZW_FAILED_NODE_NOT_REMOVED);
        }
      } else if (ev == NM_EV_TIMEOUT) {
        sl_log_error(LOG_TAG,
                     "Timed out in getting any kind of callback in"
                     " response to sending NOP to NodeID: %d\n",
                     nms.node_id_being_handled);
        on_remove_failed_status_update(ZW_FAILED_NODE_NOT_REMOVED);
        nms.state = NM_IDLE;
      } else if (ev == NM_EV_NOP_SUCCESS) {
        on_remove_failed_status_update(ZW_FAILED_NODE_NOT_REMOVED);
        nms.state = NM_IDLE;
      } else if (ev == NM_EV_ABORT) {
        // Abort the Remove Failed Operation
        sl_log_debug(LOG_TAG,
                     "Aborting Remove Failed node operation for NodeID: %d\n",
                     nms.node_id_being_handled);
        on_remove_failed_status_update(ZW_FAILED_NODE_NOT_REMOVED);
        nms.state = NM_IDLE;
      }
      break;
    case NM_SET_DEFAULT:
      if (ev == NM_EV_SET_DEFAULT_COMPLETE) {
        // Tell the Z-Wave Controller that we are done resetting.
        zwave_controller_on_reset_step_complete(
          ZWAVE_CONTROLLER_ZWAVE_NETWORK_MANAGEMENT_RESET_STEP_PRIORITY);
        //Create new security keys
        zwave_s2_create_new_network_keys();
        on_new_network(ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
        nms.state = NM_IDLE;
      }
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
      } else if (ev == NM_EV_ADD_NODE_FOUND) {  // this event is from Serialapi
                                                // from add_node_status_update
        nms.state = NM_NODE_FOUND;
        etimer_set(&nms.timer, ADD_NODE_STATUS_ADDING_END_NODE_TIMEOUT);
      }
      break;
    case NM_NODE_FOUND:
      // this event is from Serialapi from add_node_status_update
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
          // generic | specific | cmdclasses[ ]
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
        } else if (is_cc_in_nif(nms.node_info.command_class_list,
                                nms.node_info.command_class_list_length,
                                COMMAND_CLASS_SECURITY_2)
                   == SL_STATUS_OK) {
          /// S2 inclusion starts, we set the flag to S2_ADD to ensure it is S2 inclusion
          nms.flags = NMS_FLAG_S2_ADD;
          nms.state = NM_WAIT_FOR_SECURE_ADD;
          zwave_s2_start_add_node(nms.node_id_being_handled);
          zwave_controller_storage_as_set_node_s2_capable(
            nms.node_id_being_handled);
        } else if (is_cc_in_nif(nms.node_info.command_class_list,
                                nms.node_info.command_class_list_length,
                                COMMAND_CLASS_SECURITY)
                   == SL_STATUS_OK) {
          nms.state = NM_WAIT_FOR_SECURE_ADD;
          zwave_s0_start_bootstrapping(nms.node_id_being_handled,
                                       nms.is_controller);
        } else {
          // None secure inclusion is performed, therefore, we set the granted key to zero
          nms.granted_keys  = 0;
          nms.kex_fail_type = 0;
          dispatch_node_added(false);
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
          dispatch_node_added(true);
        } else if (nms.flags & NMS_FLAG_SMART_START_INCLUSION) {
          nms.state = NM_SEND_NOP;
          etimer_set(&nms.timer, SMART_START_SELF_DESTRUCT_TIMEOUT);
          break;
        } else {
          dispatch_node_added(true);
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
            zwave_s2_dsk_accept(1, nms.expected_dsk, 2);
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
            zwave_s2_dsk_accept(0, 0, 0);
          }
        } else {
          // Non-SmartStart inclusions sends the request upwards
          zwave_controller_on_dsk_report(nms.reported_dsk_blanked,
                                         nms.reported_dsk,
                                         nms.requested_keys);
        }
      } else if (ev == NM_EV_ADD_SECURITY_KEYS_SET) {
        zwave_s2_key_grant(nms.accepted_s2_bootstrapping,
                           nms.requested_keys & nms.granted_keys,
                           nms.accepted_csa);
      } else if (ev == NM_EV_ADD_SECURITY_DSK_SET) {
        zwave_s2_dsk_accept(1, nms.verified_dsk_input, 2);
        memcpy(nms.reported_dsk, nms.verified_dsk_input, 2);
      } else if (ev == NM_EV_ABORT
                 && !(nms.flags & NMS_FLAG_SMART_START_INCLUSION)) {
        // If Event is Abort and it isn't SmartStart inclusion, Reject the DSK
        zwave_s2_dsk_accept(0, nms.verified_dsk_input, 0);
      }
      break;
#if 0
//    case NM_WAIT_FOR_PROBE_BY_SIS:
//      if( ev == NM_EV_TIMEOUT ) {
//          /* rd_probe_lock(FALSE); dont unlock here, we will reset and then unlock */
//          /*Create an async application reset */
//          process_post(&zip_process, ZIP_EVENT_RESET, 0);
//          nms.state = NM_WAIT_FOR_OUR_PROBE;
//      } else if (ev  == NM_EV_FRAME_RECEIVED) {
//        ctimer_set(&nms.timer, 6000, timeout, 0);
//      }
//      break;
//    case NM_WAIT_FOR_SECURE_LEARN:
//      if(ev == NM_EV_ADD_SECURITY_KEY_CHALLENGE) {
//
//        /*Update Command classes, according to our new network state. */
//        SetPreInclusionNIF(SECURITY_SCHEME_2_ACCESS);
//
//        ZW_LEARN_MODE_SET_STATUS_FRAME_EX *f = (ZW_LEARN_MODE_SET_STATUS_FRAME_EX*) &nms.buf;
//        s2_node_inclusion_challenge_t *challenge_evt = (s2_node_inclusion_challenge_t *)event_data;
//
//
//        /*Fill in the dsk part of the answer, the rest of the answer is filled in later*/
//        memcpy(f->dsk,challenge_evt->public_key,16);
//        sec2_dsk_accept(1,f->dsk,2);
//      } else if(ev == NM_EV_SECURITY_DONE) {
//        ZW_LEARN_MODE_SET_STATUS_FRAME_EX *f = (ZW_LEARN_MODE_SET_STATUS_FRAME_EX*) &nms.buf;
//        uint32_t inclusion_flags;
//
//        inclusion_flags = *(uint32_t*) event_data;
//
//        security_init();
//        DBG_PRINTF("inclusion flags ...... %x\n",inclusion_flags);
//        f->cmdClass =
//        COMMAND_CLASS_NETWORK_MANAGEMENT_BASIC;
//        f->cmd = LEARN_MODE_SET_STATUS;
//        f->reserved = 0;
//        f->seqNo = nms.seq;
//        f->newNodeId = MyNodeID;
//        f->granted_keys = sec2_gw_node_flags2keystore_flags(inclusion_flags & 0xFF);
//        f->kexFailType = (inclusion_flags >> 16) & 0xFF;
//        f->status = (NODE_FLAG_KNOWN_BAD & inclusion_flags) ? ADD_NODE_STATUS_SECURITY_FAILED : ADD_NODE_STATUS_DONE;
//        nms.buf_len = sizeof(ZW_LEARN_MODE_SET_STATUS_FRAME_EX);
//        nms.state = NM_WAIT_FOR_MDNS;
//        /*Clear out our S0 key*/
//        if((NODE_FLAG_SECURITY0 & inclusion_flags) ==0) {
//          keystore_network_key_clear(KEY_CLASS_S0);
//        }
//        /* If we are non-securely included, we need to clear our S2 keys here.
//         * We cannot rely on the S2 security modules to do it for us since
//         * S2 bootstrapping might never started. */
//        if((NODE_FLAGS_SECURITY2 & inclusion_flags) == 0) {
//          WRN_PRINTF("Clearing all S2 keys - nonsecure inclusion or no S2 keys granted\n");
//          keystore_network_key_clear(KEY_CLASS_S2_ACCESS);
//          keystore_network_key_clear(KEY_CLASS_S2_AUTHENTICATED);
//          keystore_network_key_clear(KEY_CLASS_S2_UNAUTHENTICATED);
//        }
//        /*This is a new network, start sending mDNS goodbye messages,  */
//        /* NetworkManagement_mdns_exited will be called at some point */
//        rd_exit();
//      } else if(ev == NM_EV_LEARN_SET) {
//        ZW_LEARN_MODE_SET_FRAME* f = (ZW_LEARN_MODE_SET_FRAME*)event_data;
//        if(f->mode == ZW_SET_LEARN_MODE_DISABLE) {
//          nms.seq = f->seqNo; //Just because this was how we did in 2.2x
//          /* Aborting S2 will cause S2 to return a SECURITY_DONE with fail. */
//          sec2_abort_join();
//        }
//      }
//      break;
//    case NM_LEARN_MODE:
//      if(ev == NM_EV_TIMEOUT) {
//        if(nms.count == 0) {
//          /* We must stop Learn Mode (direct range) before starting learn mode NWI */
//          ZW_SetLearnMode(ZW_SET_LEARN_MODE_DISABLE, NULL);
//          if(nms.flags & NMS_FLAG_LEARNMODE_NWI) {
//            ZW_SetLearnMode(ZW_SET_LEARN_MODE_NWI,LearnModeStatus);
//          } else if(nms.flags & NMS_FLAG_LEARNMODE_NWE) {
//            ZW_SetLearnMode(ZW_SET_LEARN_MODE_NWE,LearnModeStatus);
//          }
//        }
//
//        if( (nms.flags & (NMS_FLAG_LEARNMODE_NWI | NMS_FLAG_LEARNMODE_NWE)) && (nms.count < 4)) {
//          if(nms.flags & NMS_FLAG_LEARNMODE_NWI) {
//            ZW_ExploreRequestInclusion();
//          } else {
//            ZW_ExploreRequestExclusion();
//          }
//
//          int delay =  CLOCK_SECOND*4 + (rand() & 0xFF);
//          ctimer_set(&nms.timer,delay, timeout, 0);
//          nms.count++;
//        } else {
//          LearnTimerExpired();
//        }
//      } else if(ev == NM_EV_LEARN_SET) {
//        ZW_LEARN_MODE_SET_FRAME* f = (ZW_LEARN_MODE_SET_FRAME*) event_data;
//
//        if (f->mode == ZW_SET_LEARN_MODE_DISABLE)
//        {
//          nms.seq = f->seqNo; //Just because this was how we did in 2.2x
//          LearnTimerExpired();
//        }
//      }
//      break;
#endif
    case NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL:
    case NM_WAITING_FOR_FAILED_NODE_REMOVAL:
      if (ev == NM_EV_REMOVE_FAILED_OK) {
        sl_log_debug(LOG_TAG,
                     "Self Destruct/Remove Failed operation "
                     "successful!\n");
        network_management_refresh_cached_node_list();
        zwave_controller_on_node_deleted(nms.node_id_being_handled);
        nms.state = NM_IDLE;
      } else if (ev == NM_EV_REMOVE_FAILED_FAIL) {
        sl_log_error(LOG_TAG,
                     "Self Destruct/Removal Failed operation "
                     "failed\n");
        if (nms.state == NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL) {
          zwave_controller_on_error(
            ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_SECURITY_FAIL);
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
          zwave_controller_on_error(
            ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_SECURITY_FAIL);
        } else {
          zwave_controller_on_error(
            ZWAVE_NETWORK_MANAGEMENT_ERROR_FAILED_NODE_REMOVE_FAIL);
        }
        nms.state = NM_IDLE;
      }
      break;
    case NM_LEARN_MODE_STARTED:
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
        etimer_stop(&nms.timer);
      }
      break;

    case NM_LEARN_MODE:
      if (ev == NM_EV_TIMEOUT) {
        if (nms.count == 0) {
          zwapi_set_learn_mode(ZW_SET_LEARN_MODE_DISABLE, NULL);
          if (nms.flags & NMS_FLAG_LEARNMODE_NWI) {
            zwapi_set_learn_mode(ZW_SET_LEARN_MODE_NWI, NULL);
          } else if (nms.flags & NMS_FLAG_LEARNMODE_NWE) {
            zwapi_set_learn_mode(ZW_SET_LEARN_MODE_NWE, NULL);
          }
        }

        if ((nms.flags & (NMS_FLAG_LEARNMODE_NWI | NMS_FLAG_LEARNMODE_NWE))
            && (nms.count < 4)) {
          if (nms.flags & NMS_FLAG_LEARNMODE_NWI) {
            sl_log_debug(LOG_TAG, "Sending explore_request_inclusion\n");
            zwapi_explore_request_inclusion();
          } else {
            sl_log_debug(LOG_TAG, "Sending explore_request_exclusion\n");
            zwapi_explore_request_exclusion();
          }
        } else {
          nms.state = NM_IDLE;
          etimer_stop(&nms.timer);
        }

        int delay_timer_learn_mode_nwi_nwe = CLOCK_SECOND * 4 + (rand() & 0xFF);
        etimer_set(&nms.timer, delay_timer_learn_mode_nwi_nwe);
        nms.count++;

      } else if (ev == NM_EV_LEARN_STOP) {
        zwapi_set_learn_mode(ZW_SET_LEARN_MODE_DISABLE, NULL);
        nms.state = NM_IDLE;
        etimer_stop(&nms.timer);
      }
      break;
    case NM_WAIT_FOR_SECURE_LEARN:
    case NM_WAIT_FOR_PROBE_BY_SIS:
    case NM_REPLACE_FAILED_REQ:
    case NM_PREPARE_SUC_INCLISION:
    case NM_WAIT_FOR_SUC_INCLUSION:
    case NM_PROXY_INCLUSION_WAIT_NIF:
      break;
  }
  sl_log_debug(LOG_TAG,
               "Exit event: %s state: %s SmartStart enabled %i\n",
               nm_event_name(ev),
               nm_state_name(nms.state),
               nms.smart_start_enabled);
  // Updating the network management state to the MQTT broker
  zwave_controller_on_state_updated(nms.state);

  if (nms.state == NM_IDLE) {
    // If we are idle, Ensure no timers are running
    etimer_stop(&nms.timer);
    // Don't get confused with previous operation data for the next operation
    nms.node_id_being_handled = 0;
    nms.flags                 = 0;
    nms.inclusion_protocol    = PROTOCOL_ZWAVE;

    // Restart SmartStart on the Z-Wave module if idle and SmartStart is enabled.
    if (nms.smart_start_enabled) {
      sl_log_debug(LOG_TAG, "Re-enabling SmartStart Add mode\n");
      zwapi_add_node_to_network(ADD_NODE_SMART_START
                                  | ADD_NODE_OPTION_NETWORK_WIDE,
                                NULL);
    }
  }
}

// void
// NetworkManagement_s0_started()
// {
//   nm_fsm_post_event(NM_EV_S0_STARTED, 0);
// }
// void NetworkManagement_start_proxy_replace(uint8_t node_id) {
//   nm_fsm_post_event(NM_EV_START_PROXY_REPLACE, &node_id);
// }

zwave_home_id_t zwave_network_management_get_home_id()
{
  return nms.cached_home_id;
}

zwave_node_id_t zwave_network_management_get_node_id()
{
  return nms.cached_local_node_id;
}

zwave_keyset_t zwave_network_management_get_granted_keys()
{
  return zwave_s2_keystore_get_assigned_keys();
}
