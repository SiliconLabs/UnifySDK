/* © 2019 Silicon Laboratories Inc. */
/******************************************************************************
 * @file zwave_network_management.c
 * @brief Implementation of zwave_network_management
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
 ******************************************************************************/

#include "zwave_network_management.h"
#include "zwave_controller_types.h"
#include "nm_state_machine.h"
#include "zwave_controller_types.h"
#include "zwapi_protocol_controller.h"
#include "process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sl_log.h"

#define LOG_TAG "zwave_network_managment"

static smartstart_event_data_t smartstart_event_data;

PROCESS_NAME(zwave_network_management_process);

void zwave_network_management_smart_start_enable(bool enable)
{
  //We update the enable state no matter what.
  nms.smart_start_enabled = enable;
  process_post(&zwave_network_management_process, NM_EV_SMART_START_ENABLE, 0);
}

zwave_network_management_state_t zwave_network_management_get_state()
{
  return nms.state;
}

sl_status_t zwave_network_management_add_node_with_dsk(
  const zwave_dsk_t dsk,
  zwave_keyset_t granted_keys,
  zwave_protocol_t preferred_inclusion)
{
  if (nms.state != NM_IDLE) {
    sl_log_info(LOG_TAG,
                "Network management is busy. "
                "Ignoring Add node with DSK request.\n");
    return SL_STATUS_BUSY;
  }
  memcpy(smartstart_event_data.dsk, dsk, sizeof(zwave_dsk_t));
  smartstart_event_data.granted_keys        = granted_keys;
  smartstart_event_data.preferred_inclusion = preferred_inclusion;
  process_post(&zwave_network_management_process,
               NM_EV_NODE_ADD_SMART_START,
               (void *)&smartstart_event_data);
  return SL_STATUS_OK;
}

sl_status_t zwave_network_management_set_default()
{
  if (nms.state != NM_IDLE) {
    sl_log_info(LOG_TAG,
                "Network management is busy. "
                "Ignoring Set Default request.\n");
    return SL_STATUS_BUSY;
  }
  sl_log_info(
    LOG_TAG,
    "Reset step: Initiating a Z-Wave API Set Default / Reset operation\n");
  process_post(&zwave_network_management_process, NM_EV_SET_DEFAULT, 0);
  return SL_STATUS_OK;
}

sl_status_t zwave_network_management_remove_failed(zwave_node_id_t node_id)
{
  if (nms.state != NM_IDLE) {
    sl_log_info(LOG_TAG,
                "Network management is busy. "
                "Ignoring Remove Failed node request.\n");
    return SL_STATUS_BUSY;
  }
  sl_log_info(LOG_TAG,
              "Initiating a Remove Failed operation for NodeID: %d\n",
              node_id);
  nms.node_id_being_handled = node_id;
  process_post(&zwave_network_management_process, NM_EV_REMOVE_FAILED, 0);
  return SL_STATUS_OK;
}

sl_status_t zwave_network_management_remove_node()
{
  if (nms.state != NM_IDLE) {
    sl_log_info(LOG_TAG,
                "Network management is busy. "
                "Ignoring Remove node request.\n");
    return SL_STATUS_BUSY;
  }
  sl_log_info(LOG_TAG, "Initiating a Z-Wave Network Exclusion\n");
  process_post(&zwave_network_management_process, NM_EV_NODE_REMOVE, 0);
  return SL_STATUS_OK;
}

static sl_status_t zwave_network_management_stop_add_mode()
{
  // Checking if the network managemnt operation is in NM_WAITING_FOR_ADD state,
  // and if it does, we stop adding process before Z-Wave Serial API sends
  // ADD_NODE_STATUS_NODE_FOUND event.
  if (nms.state == NM_WAITING_FOR_ADD) {
    sl_log_info(LOG_TAG, "Aborting a Z-Wave add node operation\n");
    process_post(&zwave_network_management_process, NM_EV_ABORT, 0);
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

sl_status_t zwave_network_management_abort()
{
  if (zwave_network_management_stop_add_mode() == SL_STATUS_OK) {
    return SL_STATUS_OK;
  }

  if (nms.state != NM_IDLE) {
    sl_log_info(LOG_TAG,
                "Aborting ongoing Z-Wave Network Management operation\n");
    process_post(&zwave_network_management_process, NM_EV_ABORT, 0);
    return SL_STATUS_OK;
  }

  return SL_STATUS_IDLE;
}

sl_status_t zwave_network_management_add_node()
{
  if (nms.state != NM_IDLE) {
    sl_log_info(LOG_TAG,
                "Network management is busy. "
                "Ignoring Add node request.\n");
    return SL_STATUS_BUSY;
  }
  sl_log_info(LOG_TAG, "Initiating a Z-Wave Network Inclusion\n");
  process_post(&zwave_network_management_process, NM_EV_NODE_ADD, 0);
  return SL_STATUS_OK;
}

sl_status_t zwave_network_management_keys_set(bool accept,
                                              bool csa,
                                              zwave_keyset_t granted_keys)
{
  if (nms.state != NM_WAIT_FOR_SECURE_ADD) {
    sl_log_error(
      LOG_TAG,
      " The Network Management S2 key set operation will not be performed "
      " since Network Management is not in the NM_WAIT_FOR_SECURE_ADD "
      "state.\n");
    return SL_STATUS_BUSY;
  }
  nms.accepted_s2_bootstrapping = accept;
  nms.granted_keys              = granted_keys;
  nms.accepted_csa              = csa;
  process_post(&zwave_network_management_process,
               NM_EV_ADD_SECURITY_KEYS_SET,
               0);
  return SL_STATUS_OK;
}

sl_status_t zwave_network_management_dsk_set(zwave_dsk_t dsk)
{
  if (nms.state != NM_WAIT_FOR_SECURE_ADD) {
    return SL_STATUS_BUSY;
  }
  memcpy(nms.verified_dsk_input, dsk, sizeof(zwave_dsk_t));
  process_post(&zwave_network_management_process,
               NM_EV_ADD_SECURITY_DSK_SET,
               0);
  return SL_STATUS_OK;
}

sl_status_t zwave_network_management_learn_mode(uint8_t intent)
{
  nms.learn_mode_intent = intent;
  if (nms.learn_mode_intent == ZW_SET_LEARN_MODE_DISABLE
      && (nms.state == NM_LEARN_MODE || nms.state == NM_LEARN_MODE_STARTED)) {
    sl_log_info(LOG_TAG, "Stoping Learn Mode Operation.\n");
    process_post(&zwave_network_management_process, NM_EV_LEARN_STOP, 0);
    return SL_STATUS_OK;
  } else if (nms.state != NM_IDLE) {
    sl_log_info(LOG_TAG,
                "Network management is busy. "
                "Ignoring Learn Mode request.\n");
    return SL_STATUS_BUSY;
  }

  process_post(&zwave_network_management_process, NM_EV_LEARN_SET, 0);
  return SL_STATUS_OK;
}

bool zwave_network_management_is_zpc_sis()
{
  uint8_t zpc_capabilities = zwapi_get_controller_capabilities();
  return (zpc_capabilities & CONTROLLER_NODEID_SERVER_PRESENT);
}

sl_status_t zwave_network_management_request_node_neighbor_discovery(
  zwave_node_id_t node_id)
{
  if (nms.state != NM_IDLE) {
    return SL_STATUS_BUSY;
  }

  process_post(&zwave_network_management_process,
               NM_EV_REQUEST_NODE_NEIGHBOR_REQUEST,
               (void *)(intptr_t)node_id);

  return SL_STATUS_OK;
}

sl_status_t zwave_network_management_start_proxy_inclusion(
  zwave_node_id_t node_id, zwave_node_info_t nif, uint8_t inclusion_step)
{
  if (nms.state != NM_IDLE) {
    return SL_STATUS_BUSY;
  }
  nms.node_id_being_handled = node_id;
  nms.flags                 = 0;
  nms.node_info             = nif;
  nms.proxy_inclusion_step  = inclusion_step;
  process_post(&zwave_network_management_process,
               NM_EV_START_PROXY_INCLUSION,
               0);
  return SL_STATUS_OK;
}

sl_status_t zwave_network_management_assign_return_route(
  zwave_node_id_t node_id, zwave_node_id_t destination_node_id)
{
  sl_status_t status
    = zwapi_assign_return_route(node_id, destination_node_id, NULL);
  return status;
}

bool zwave_network_management_is_busy()
{
  if (nms.state != NM_IDLE) {
    return true;
  }
  return false;
}
#if 0
// #define NETWORK_MANAGEMENT_TIMEOUT 2*6000
// #define SMART_START_SELF_DESTRUCT_TIMEOUT (3*CLOCK_SECOND)
//
// /* Defer restarting SmartStart Add mode after including one SmartStart node until this time
//  * has elapsed. */
// #define SMART_START_MIDDLEWARE_PROBE_TIMEOUT (9 * CLOCK_SECOND)
//
// extern uint8_t ecdh_dynamic_key[32];
// extern void crypto_scalarmult_curve25519_base(uint8_t *q, const uint8_t *n);
// extern void keystore_public_key_debug_print(void);
//
// /* See CC.0034.01.08.11.001 */
// #define NM_FAILED_NODE_REMOVE_FAIL 0x02
// #define NM_FAILED_NODE_REMOVE_DONE 0x01
// #define NM_FAILED_NODE_NOT_FOUND 0x00
//
// /** Internal Network Management events.
//  * \ingroup NW_CMD_handler
//  */
//
// static void
// RemoveNodeStatusUpdate(LEARN_INFO* inf);
// static void
// SecureInclusionDone(int status);

// /* This is a lock which prevents reactivation of SmartStart inclusion
//  * until middleware_probe_timeout() has triggered */
// int waiting_for_middleware_probe = FALSE;
//
// /* SmartStart middleware probe timer */
// static struct ctimer ss_timer;
//

// /**
//  * Timeout for remove node.
//  */

// #if 0
//
//
// /**
//  * Return true is this is a clean network containing only the ZIP router and
//  * if this is a new network compared to what we have en RAM
//  */
// static void
// isCleanNetwork(BOOL* clean_network, BOOL* new_network)
// {
//   BYTE ver, capabilities, len;
//   BYTE node_list[MAX_NODEMASK_LENGTH];
//   BYTE c, v;
//   DWORD h;
//   BYTE n;
//   BYTE i;
//
//   MemoryGetID((BYTE*) &h, &n);
//   SerialAPI_GetInitData(&ver, &capabilities, &len, node_list, &c, &v);
//
//   *new_network = (h != homeID);
//
//   node_list[(n - 1) >> 3] &= ~(1 << ((n - 1) & 0x7));
//   for (i = 0; i < MAX_NODEMASK_LENGTH; i++)
//   {
//     if (node_list[i])
//     {
//       *clean_network = FALSE;
//       return;
//     }
//   }
//
//   *clean_network = TRUE;
// }
//

// /**
//  * This is where network management is actually performed.
//  */
// static command_handler_codes_t
// NetworkManagementAction(ZW_APPLICATION_TX_BUFFER* pCmd, BYTE bDatalen)
// {
// //  memset(&nms.buf, 0, sizeof(nms.buf));
// //  nms.buf.ZW_Common.cmdClass = pCmd->ZW_Common.cmdClass;
//   switch (pCmd->ZW_Common.cmdClass)
//   {
//   /* case COMMAND_CLASS_NETWORK_MANAGEMENT_PROXY was here */
//   case COMMAND_CLASS_NETWORK_MANAGEMENT_BASIC:
//     switch (pCmd->ZW_Common.cmd)
//     {
//     case NODE_INFORMATION_SEND:
//         /* If the the Gateway is asked to send a broadcast NIF,it will report the supported command classes
//          * depending on the granted security class:
//          * ->If the Gateway is granted S2 keys, the Gateway will advertise the command classes
//          *   that are supported non securely.
//          * ->If the Gateway is granted no key, the Gateway will advertise non secure plus
//          *   net scheme (“highest granted key”) supported command classes.
//          */
//       if(nms.state != NM_IDLE) return COMMAND_BUSY; //TODO move into fsm;
//
//       if(ZW_SendNodeInformation(pCmd->ZW_NodeInformationSendFrame.destinationNodeId,
//           pCmd->ZW_NodeInformationSendFrame.txOptions, __ResetState)) {
//             nms.state = NM_SENDING_NODE_INFO;
//       } else {
//           __ResetState(TRANSMIT_COMPLETE_FAIL);
//       }
//       break;
//     case NETWORK_UPDATE_REQUEST:
//       if(nms.state != NM_IDLE) return COMMAND_BUSY; //TODO move into fsm;
//
//       if (ZW_RequestNetWorkUpdate(NetworkUpdateCallback))
//       {
//         /* Asking SUC/SIS. */
//         nms.state = NM_NETWORK_UPDATE;
//       }
//       else
//       {
//         /*I'm the SUC/SIS or i don't know the SUC/SIS*/
//         if (ZW_GetSUCNodeID() > 0)
//         {
//           NetworkUpdateCallback(ZW_SUC_UPDATE_DONE);
//         }
//         else
//         {
//           NetworkUpdateCallback(ZW_SUC_UPDATE_DISABLED);
//         }
//       }
//       break;
//     default:
//       return COMMAND_NOT_SUPPORTED;
//     }
//     break;
//   case COMMAND_CLASS_NETWORK_MANAGEMENT_INCLUSION:
//     /*If there is neither a SIS or we are primary controller, we cannot perform inclusion*/
//     if (!((ZW_GetControllerCapabilities() & CONTROLLER_NODEID_SERVER_PRESENT) || ZW_IsPrimaryCtrl()))
//       return COMMAND_NOT_SUPPORTED;
//
//     switch (pCmd->ZW_Common.cmd)
//     {
//     case NODE_ADD:
//       {
//         uint8_t mode = ADD_NODE_ANY;
//
//         if (!(pCmd->ZW_NodeAddFrame.txOptions & TRANSMIT_OPTION_LOW_POWER))
//         {
//           mode |= ADD_NODE_OPTION_NORMAL_POWER;
//         }
//
//         if (pCmd->ZW_NodeAddFrame.txOptions & TRANSMIT_OPTION_EXPLORE)
//         {
//           mode |= ADD_NODE_OPTION_NETWORK_WIDE;
//         }
//
//         nms.txOptions = TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE | pCmd->ZW_NodeAddFrame.txOptions;
//
//         if (pCmd->ZW_NodeAddFrame.mode == ADD_NODE_STOP)
//         {
//           DBG_PRINTF("Add node stop\n");
//           nm_fsm_post_event(NM_NODE_ADD_STOP, 0);
//         }
//         else if (pCmd->ZW_NodeAddFrame.mode == ADD_NODE_ANY)
//         {
//           nm_fsm_post_event(NM_EV_NODE_ADD, &mode);
//         }
//         else if (pCmd->ZW_NodeAddFrame.mode == ADD_NODE_ANY_S2)
//         {
//           nm_fsm_post_event(NM_EV_NODE_ADD_S2, &mode);
//         }
//       }
//       break;
//     case FAILED_NODE_REPLACE:
//       {
//         ZW_FAILED_NODE_REPLACE_FRAME* f = (ZW_FAILED_NODE_REPLACE_FRAME*) pCmd;
//
//         if (f->mode == START_FAILED_NODE_REPLACE)
//         {
//           nm_fsm_post_event(NM_EV_REPLACE_FAILED_START,pCmd);
//         } else if(f->mode == START_FAILED_NODE_REPLACE_S2)
//         {
//           nm_fsm_post_event(NM_EV_REPLACE_FAILED_START_S2,pCmd);
//         }
//         else if (f->mode == STOP_FAILED_NODE_REPLACE)
//         {
//           nm_fsm_post_event(NM_EV_REPLACE_FAILED_STOP,pCmd);
//         }
//       }
//       break;
//     case NODE_NEIGHBOR_UPDATE_REQUEST:
//       {
//         if(nms.state != NM_IDLE) return COMMAND_BUSY; //TODO move into fsm;
//
//         ZW_NODE_NEIGHBOR_UPDATE_REQUEST_FRAME *f = (ZW_NODE_NEIGHBOR_UPDATE_REQUEST_FRAME *) pCmd;
//         nms.state = NM_WAITING_FOR_NODE_NEIGH_UPDATE;
//         /* Using a timeout here is a workaround for a defect in the protocol
//          * (see SWPROT-3666) causing it to not always activate the
//          * RequestNodeNeighborUpdateStatus callback when the network has FLIRS
//          * devices.
//          */
//         clock_time_t tout = rd_calculate_inclusion_timeout(TRUE);
//         ctimer_set(&nms.timer, tout, timeout, 0);
//         ZW_RequestNodeNeighborUpdate(f->nodeId, RequestNodeNeighborUpdateStatus);
//       }
//       break;
//     case RETURN_ROUTE_ASSIGN:
//       {
//         if(nms.state != NM_IDLE) return COMMAND_BUSY; //TODO move into fsm;
//
//         ZW_RETURN_ROUTE_ASSIGN_FRAME *f = (ZW_RETURN_ROUTE_ASSIGN_FRAME*) pCmd;
//         nms.state = NM_WAITING_FOR_RETURN_ROUTE_ASSIGN;
//         if (!ZW_AssignReturnRoute(f->sourceNodeId, f->destinationNodeId, AssignReturnRouteStatus))
//         {
//           AssignReturnRouteStatus(TRANSMIT_COMPLETE_FAIL);
//         }
//       }
//       break;
//     case RETURN_ROUTE_DELETE:
//       {
//         if(nms.state != NM_IDLE) return COMMAND_BUSY; //TODO move into fsm;
//
//         ZW_RETURN_ROUTE_DELETE_FRAME *f = (ZW_RETURN_ROUTE_DELETE_FRAME*) pCmd;
//         nms.state = NM_WAITING_FOR_RETURN_ROUTE_DELETE;
//         if(ZW_DeleteReturnRoute(f->nodeId, DeleteReturnRouteStatus) != TRUE) {
//            DeleteReturnRouteStatus(TRANSMIT_COMPLETE_FAIL);
//         }
//       }
//       break;
//     }
//     break;
//
//   default:
//     return CLASS_NOT_SUPPORTED;
//   } /* switch(COMMAND_CLASS)*/
//   return COMMAND_HANDLED;
// }
//

#endif

#if 0

// static void
// middleware_probe_timeout(void* none)
// {
//   waiting_for_middleware_probe = FALSE;
//   if (nms.state == NM_IDLE) {
//      process_post(&zip_process, ZIP_EVENT_COMPONENT_DONE, &extend_middleware_probe_timeout);
//   }
//   NetworkManagement_smart_start_init_if_pending();
// }
//
// void extend_middleware_probe_timeout(void)
// {
//   if (waiting_for_middleware_probe) {
//     ctimer_set(&ss_timer, SMART_START_MIDDLEWARE_PROBE_TIMEOUT, middleware_probe_timeout, 0);
//   }
// }
//
#endif
