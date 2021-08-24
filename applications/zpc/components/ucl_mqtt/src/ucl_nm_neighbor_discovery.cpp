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

#include "sl_log.h"
#include "sl_status.h"
#include "zwave_controller_callbacks.h"
#include "zwave_network_management.h"
#include "zwave_utils.h"
#include "sys/ctimer.h"
#include <deque>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
// Defines and types
////////////////////////////////////////////////////////////////////////////////
#define LOG_TAG "ucl_nm_neighbor_discovery"
/// setting a 5 minutes timeout to tackle a defect in protocol
/// which may not send a callback message to the host application.
/// The value is estimated considering that neigbour discovery
/// is triggered in a large network (i.e., 232 nodes)
#define UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_DEFAULT_TIMEOUT 3000000
/**
 * @brief All possible values for Node Request Node Neighbor Discovery Statuses
 *        \ref zwapi_request_neighbor_update() callback status values       
 *         i.e., UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_STARTED = 0x21 
 */
typedef enum {
  UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_STARTED = 33,
  UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_COMPLETED,
  UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_FAILED,
  UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_TIMEOUT,
  UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_NOT_SUPPORTED = 255,
} ucl_network_management_neighbor_discovery_t;

static struct ctimer request_neighbor_update_ctimer;
static bool on_going_requested_node_neighbor_update = false;
static std::deque<zwave_node_id_t> requested_node_neighbor_update_list;
static void on_timeout_requested_node_neighbor_update(void *data);

static void ucl_nm_request_node_neighbor_update_trigger()
{
  if ((zwave_network_management_get_state() == NM_IDLE)
      && (!on_going_requested_node_neighbor_update)) {
    on_going_requested_node_neighbor_update = true;
    zwave_network_management_request_node_neighbor_discovery(
      requested_node_neighbor_update_list.front());
    ctimer_set(&request_neighbor_update_ctimer,
               UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_DEFAULT_TIMEOUT,
               on_timeout_requested_node_neighbor_update,
               0);
  }
}

static void ucl_nm_request_node_neighbor_update_callback(uint8_t status)
{
  switch (status) {
    case UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_STARTED:
      sl_log_info(LOG_TAG,
                  "Node(%i) Neighbor Discovery is started.",
                  requested_node_neighbor_update_list.front());
      break;
    case UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_FAILED:
      sl_log_info(LOG_TAG,
                  "Node(%i) Neighbor Discovery is failed.",
                  requested_node_neighbor_update_list.front());
      break;
    case UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_COMPLETED:
      sl_log_info(LOG_TAG,
                  "Node(%i) Neighbor Discovery is completed",
                  requested_node_neighbor_update_list.front());
      break;
    case UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_NOT_SUPPORTED:
      sl_log_info(LOG_TAG,
                  "Node(%i) Neighbor Discovery is not supported.",
                  requested_node_neighbor_update_list.front());
      break;
    case UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_TIMEOUT:
      sl_log_info(LOG_TAG,
                  "Node(%i ) Neighbor Discovery request timeout.",
                  requested_node_neighbor_update_list.front());
      break;
  }

  if (status != UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_STARTED) {
    requested_node_neighbor_update_list.pop_front();
    on_going_requested_node_neighbor_update = false;
    ctimer_stop(&request_neighbor_update_ctimer);
    if (!requested_node_neighbor_update_list.empty()) {
      ucl_nm_request_node_neighbor_update_trigger();
    }
  }
}

static void ucl_nm_neighbor_discovery_on_state_updated(
  zwave_network_management_state_t nm_state)
{
  if (nm_state == NM_IDLE) {
    if (!requested_node_neighbor_update_list.empty()) {
      ucl_nm_request_node_neighbor_update_trigger();
    }
  }
}

static const zwave_controller_callbacks_t ucl_nm_neighbor_discovery_callbacks
  = {
    .on_state_updated           = ucl_nm_neighbor_discovery_on_state_updated,
    .on_request_neighbor_update = ucl_nm_request_node_neighbor_update_callback,
};

static void on_timeout_requested_node_neighbor_update(void *data)
{
  ucl_nm_request_node_neighbor_update_callback(
    UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_TIMEOUT);
}

///////////////////////////////////////////////////////////////////////////////
// Publication functions
//////////////////////////////////////////////////////////////////////////////
void ucl_nm_trigger_node_neighbor_update(zwave_node_id_t node_id)
{
  zwave_protocol_t node_protocol = get_protocol(node_id);
  if (node_protocol == PROTOCOL_ZWAVE_LONG_RANGE){
    //If the Node is operating on the Z-Wave Long Range PHY/MAC/NWK,
    // we do not perform neighbor discovery.
    return;
  }
  std::deque<zwave_node_id_t>::iterator it
    = find(requested_node_neighbor_update_list.begin(),
           requested_node_neighbor_update_list.end(),
           node_id);
  if (it == requested_node_neighbor_update_list.end()) {
    requested_node_neighbor_update_list.push_back(node_id);
    ucl_nm_request_node_neighbor_update_trigger();
  }
}

void ucl_nm_neighbor_discovery_init()
{
  zwave_controller_register_callbacks(&ucl_nm_neighbor_discovery_callbacks);
}