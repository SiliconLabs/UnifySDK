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
#include "zwave_network_management_process.h"
#include "zwave_network_management_return_route_queue.h"
#include "nm_state_machine.h"

#include "zwave_controller_types.h"
#include "zwapi_protocol_controller.h"
#include "zwave_s2_keystore.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sl_log.h"
#define LOG_TAG "zwave_network_managment"

static smartstart_event_data_t smartstart_event_data;

void zwave_network_management_enable_smart_start_add_mode(bool enabled)
{
  //We update the enable state no matter what.
  nms.smart_start_add_mode_enabled = enabled;
  process_post(&zwave_network_management_process, NM_EV_SMART_START_ENABLE, 0);
}

void zwave_network_management_enable_smart_start_learn_mode(bool enabled)
{
  // Tell NMS to use SmartStart learn mode when possible.
  nms.smart_start_learn_mode_enabled = enabled;
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
  // If assigning return routes, just abort this
  if (nms.state == NM_ASSIGNING_RETURN_ROUTE) {
    zwave_network_management_return_route_clear_queue();
    nms.state = NM_IDLE;
  }
  if (nms.state != NM_IDLE) {
    sl_log_info(LOG_TAG,
                "Network management is busy. "
                "Ignoring Set Default request.\n");
    return SL_STATUS_BUSY;
  }
  sl_log_info(LOG_TAG,
              "Reset step: Initiating a Z-Wave "
              "API Set Default / Reset operation\n");

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
              "Initiating a Remove Offline operation for NodeID: %d\n",
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
  // Checking if the network management operation is in NM_WAITING_FOR_ADD state,
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
      "The Network Management S2 key set operation will not be performed "
      "since Network Management is not in the NM_WAIT_FOR_SECURE_ADD "
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

sl_status_t zwave_network_management_learn_mode(zwave_learn_mode_t mode)
{
  if (mode == ZWAVE_NETWORK_MANAGEMENT_LEARN_NONE) {
    if (nms.state == NM_LEARN_MODE) {
      process_post(&zwave_network_management_process, NM_EV_ABORT, 0);
    }
    return SL_STATUS_OK;
  }

  if (nms.state != NM_IDLE) {
    sl_log_info(LOG_TAG,
                "Network management is busy. "
                "Ignoring Learn Mode request.\n");
    return SL_STATUS_BUSY;
  }

  nms.learn_mode_intent = mode;
  process_post(&zwave_network_management_process, NM_EV_LEARN_SET, 0);
  return SL_STATUS_OK;
}

bool zwave_network_management_is_zpc_sis()
{
  const bool we_have_the_sis_role
    = ((nms.controller_capabilities_bitmask & CONTROLLER_IS_SECONDARY) == 0)
      && (nms.controller_capabilities_bitmask
          & CONTROLLER_NODEID_SERVER_PRESENT)
      && (nms.controller_capabilities_bitmask & CONTROLLER_IS_SUC);

  return we_have_the_sis_role;
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
  assign_return_route_t assign_return_request
    = {.node_id             = node_id,
       .destination_node_id = destination_node_id,
       .in_progress         = false};
  return zwave_network_management_return_route_add_to_queue(
    &assign_return_request);
}

bool zwave_network_management_is_busy()
{
  if (nms.state != NM_IDLE) {
    return true;
  }
  return false;
}

zwave_home_id_t zwave_network_management_get_home_id()
{
  return nms.cached_home_id;
}

zwave_node_id_t zwave_network_management_get_node_id()
{
  return nms.cached_local_node_id;
}

void zwave_network_management_get_network_node_list(zwave_nodemask_t node_list)
{
  memcpy(node_list, nms.cached_node_list, sizeof(zwave_nodemask_t));
}

zwave_keyset_t zwave_network_management_get_granted_keys()
{
  return zwave_s2_keystore_get_assigned_keys();
}
