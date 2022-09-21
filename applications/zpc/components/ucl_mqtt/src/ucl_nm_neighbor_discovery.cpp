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
#include "ucl_nm_neighbor_discovery.h"

// ZPC Components
#include "zwave_controller_callbacks.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_network_management.h"
#include "zwave_utils.h"

// Unify Components
#include "sl_log.h"
#include "sl_status.h"
#include "attribute_resolver.h"
#include "attribute_store_helper.h"
#include "sys/ctimer.h"

// Generic includes
#include <deque>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
// Defines and types
////////////////////////////////////////////////////////////////////////////////
#define LOG_TAG "ucl_nm_neighbor_discovery"

static struct ctimer request_neighbor_update_ctimer;
static bool on_going_requested_node_neighbor_update = false;
static std::deque<zwave_node_id_t> requested_node_neighbor_update_list;
static void on_timeout_requested_node_neighbor_update(void *data);

static void ucl_nm_request_node_neighbor_update_process_next()
{
  if ((!on_going_requested_node_neighbor_update)
      && (false == requested_node_neighbor_update_list.empty())
      && (zwave_network_management_get_state() == NM_IDLE)) {
    sl_status_t status
      = zwave_network_management_request_node_neighbor_discovery(
        requested_node_neighbor_update_list.front());

    if (SL_STATUS_OK == status) {
      on_going_requested_node_neighbor_update = true;
      ctimer_set(&request_neighbor_update_ctimer,
                 UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_DEFAULT_TIMEOUT,
                 on_timeout_requested_node_neighbor_update,
                 0);
    }
  }
}

static void ucl_nm_request_node_neighbor_update_callback(uint8_t status)
{
  if (false == on_going_requested_node_neighbor_update) {
    sl_log_debug(LOG_TAG,
                 "Got an unexpected status update callback. "
                 "May be be due to a previous abort or timeout. Ignoring.");
    return;
  }

  switch (status) {
    case UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_STARTED:
      sl_log_info(LOG_TAG,
                  "NodeID %i Neighbor Discovery has started.",
                  requested_node_neighbor_update_list.front());
      break;
    case UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_FAILED:
      sl_log_info(LOG_TAG,
                  "NodeID %i Neighbor Discovery has failed.",
                  requested_node_neighbor_update_list.front());
      break;
    case UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_COMPLETED:
      sl_log_info(LOG_TAG,
                  "NodeID %i Neighbor Discovery is completed",
                  requested_node_neighbor_update_list.front());
      break;
    case UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_NOT_SUPPORTED:
      sl_log_info(LOG_TAG,
                  "NodeID %i Neighbor Discovery is not supported.",
                  requested_node_neighbor_update_list.front());
      break;
    default:
      sl_log_warning(LOG_TAG,
                     "Unknown status (%d) for NodeID %i Neighbor Discovery. "
                     "Considering it failed.",
                     status,
                     requested_node_neighbor_update_list.front());
  }

  if (status != UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_STARTED) {
    requested_node_neighbor_update_list.pop_front();
    on_going_requested_node_neighbor_update = false;
    ctimer_stop(&request_neighbor_update_ctimer);
    ucl_nm_request_node_neighbor_update_process_next();
  }
}

/**
 * @brief Will attempt to request network management to get the node to
 * find its neigbor when resolution is resumed.
 *
 * @param node_id_node  Attribute store node for the NodeID.
 */
static void ucl_nm_request_node_neighbor_on_node_resolution_resumed(
  attribute_store_node_t node_id_node)
{
  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));

  if ((zwave_network_management_get_state() == NM_IDLE)
      && (!on_going_requested_node_neighbor_update)) {
    sl_status_t status
      = zwave_network_management_request_node_neighbor_discovery(node_id);

    if (SL_STATUS_OK == status) {
      requested_node_neighbor_update_list.push_front(node_id);
      on_going_requested_node_neighbor_update = true;
      ctimer_set(&request_neighbor_update_ctimer,
                 UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_DEFAULT_TIMEOUT,
                 on_timeout_requested_node_neighbor_update,
                 nullptr);
      attribute_resolver_clear_resolution_resumption_listener(
        node_id_node,
        &ucl_nm_request_node_neighbor_on_node_resolution_resumed);
      return;
    }
  }
}

static void ucl_nm_neighbor_discovery_on_state_updated(
  zwave_network_management_state_t nm_state)
{
  if (nm_state == NM_IDLE) {
    ucl_nm_request_node_neighbor_update_process_next();
  }
}

static const zwave_controller_callbacks_t ucl_nm_neighbor_discovery_callbacks
  = {
    .on_state_updated           = ucl_nm_neighbor_discovery_on_state_updated,
    .on_request_neighbor_update = ucl_nm_request_node_neighbor_update_callback,
};

static void on_timeout_requested_node_neighbor_update(void *data)
{
  // Consider it failed at that point.
  ucl_nm_request_node_neighbor_update_callback(
    UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_FAILED);
}

///////////////////////////////////////////////////////////////////////////////
// Public functions
//////////////////////////////////////////////////////////////////////////////
void ucl_nm_trigger_node_neighbor_update(zwave_node_id_t node_id)
{
  if (zwave_get_inclusion_protocol(node_id) == PROTOCOL_ZWAVE_LONG_RANGE) {
    // If the Node is operating on the Z-Wave Long Range PHY/MAC/NWK,
    // we do not perform neighbor discovery.
    return;
  }

  if (zwave_get_operating_mode(node_id) == OPERATING_MODE_NL) {
    // We have to catch a wake up period for this.
    attribute_store_node_t node_id_node
      = attribute_store_network_helper_get_zwave_node_id_node(node_id);

    attribute_resolver_set_resolution_resumption_listener(
      node_id_node,
      &ucl_nm_request_node_neighbor_on_node_resolution_resumed);
    return;
  }

  std::deque<zwave_node_id_t>::iterator it
    = find(requested_node_neighbor_update_list.begin(),
           requested_node_neighbor_update_list.end(),
           node_id);
  if (it == requested_node_neighbor_update_list.end()) {
    requested_node_neighbor_update_list.push_back(node_id);
    ucl_nm_request_node_neighbor_update_process_next();
  }
}

void ucl_nm_neighbor_discovery_init()
{
  zwave_controller_register_callbacks(&ucl_nm_neighbor_discovery_callbacks);
  requested_node_neighbor_update_list.clear();
  on_going_requested_node_neighbor_update = false;
}