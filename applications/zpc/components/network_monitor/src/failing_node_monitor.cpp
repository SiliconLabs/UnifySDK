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
// Includes from this component
#include "failing_node_monitor.h"
#include "network_monitor_utils.h"

// Interfaces
#include "attribute_store_defined_attribute_types.h"


// ZPC components
#include "zwave_controller_utils.h"

// Unify components
#include "sl_log.h"
#include "attribute_timeouts.h"
#include "attribute_store_helper.h"
#include "unify_dotdot_attribute_store_node_state.h"

// Type aliasing
constexpr attribute_store_type_t PING_INTERVAL_ATTRIBUTE
  = ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL;

constexpr char LOG_TAG[] = "failing_node_monitor";

// Private variables
namespace
{
// boolean indicating if we qeueued a NOP and are waiting for a callback.
bool nop_queued = false;
}  // namespace

// Forward declarations
static void on_send_nop_completed(uint8_t status,
                                  const zwapi_tx_report_t *tx_info,
                                  void *user);

////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Puts a NOP in the Tx Queue for the first NodeID in the list.
 *
 */
static void queue_nop_to_failing_node(attribute_store_node_t node_id_node)
{
  if (nop_queued == true) {
    zwave_node_id_t node_id = 0;
    attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));
    // Do not add more than 1 frame at a time in the Tx Queue.
    // Wait for the duration of the discard timeout before retrying.
    sl_log_debug(LOG_TAG,
                 "Applying back-off before queueing the "
                 "next NOP for NodeID: %d.",
                 node_id);
    attribute_timeout_set_callback(node_id_node,
                                   NOP_REQUEUE_BACK_OFF_MS,
                                   &queue_nop_to_failing_node);
    return;
  }

  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));

  sl_status_t send_status
    = zwave_send_nop_to_node(node_id,
                             ZWAVE_TX_QOS_MIN_PRIORITY,
                             NOP_DISCARD_TIMEOUT_MS,
                             &on_send_nop_completed,
                             reinterpret_cast<void *>(node_id_node));

  if (SL_STATUS_OK == send_status) {
    nop_queued = true;
  } else {
    sl_log_debug(LOG_TAG,
                 "Tx Queue rejected NOP frame to NodeID %d. "
                 "Applying backing off before trying again.",
                 node_id);
    attribute_timeout_set_callback(node_id_node,
                                   NOP_REQUEUE_BACK_OFF_MS,
                                   &queue_nop_to_failing_node);
  }
}

/**
 * @brief Callback for when the Tx Queue has processed our NOP.
 *
 * @param status      Refer to @ref on_zwave_tx_send_data_complete_t
 * @param tx_info     Refer to @ref on_zwave_tx_send_data_complete_t
 * @param user        Refer to @ref on_zwave_tx_send_data_complete_t
 */
static void
  on_send_nop_completed([[maybe_unused]] uint8_t status,
                        [[maybe_unused]] const zwapi_tx_report_t *tx_info,
                        void *user)
{
  // Tell the component that we can queue new NOPs.
  nop_queued = false;

  auto node_id_node
    = static_cast<attribute_store_node_t>(reinterpret_cast<intptr_t>(user));

  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));
  sl_log_debug(LOG_TAG, "Send NOP Completed for NodeID: %d.", node_id);

  // Increment the Ping interval for the node that we just NOP'ed
  attribute_store_node_t ping_interval_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             PING_INTERVAL_ATTRIBUTE,
                                             0);
  clock_time_t ping_interval = 0;
  attribute_store_get_reported(ping_interval_node,
                               &ping_interval,
                               sizeof(ping_interval));
  if (ping_interval == 0) {
    return;
  }

  if (network_monitor_get_operating_mode(node_id_node) == OPERATING_MODE_AL) {
    ping_interval *= AL_NODE_PING_TIME_FACTOR;
  } else {
    ping_interval *= FL_NODE_PING_TIME_FACTOR;
  }
  if (ping_interval > MAX_PING_TIME_INTERVAL) {
    ping_interval = MAX_PING_TIME_INTERVAL;
  }

  sl_log_debug(LOG_TAG,
               "Scheduling NOP command in %lu ms for NodeID: %d.",
               ping_interval,
               node_id);
  attribute_store_set_reported(ping_interval_node,
                               &ping_interval,
                               sizeof(ping_interval));
  attribute_timeout_set_callback(node_id_node,
                                 ping_interval,
                                 &queue_nop_to_failing_node);
}

/**
 * @brief Verifies the network status value and tells if the node is failing.
 *
 * @param network_status_node   Network Status node
 * @return true if the node is failing and should be monitored, false otherwise.
 * @return false
 */
static bool is_node_failing(attribute_store_node_t network_status_node)
{
  // Assume the node is just included if we cannot read the network status
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  switch (network_status) {
    case ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE:
    case ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL:
      return true;

    // All the rest is considered as not failing.
    default:
      return false;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
////////////////////////////////////////////////////////////////////////////////
static void on_network_status_update(attribute_store_node_t network_status_node,
                                     attribute_store_change_t change)
{
  if (change == ATTRIBUTE_CREATED) {
    return;
  }

  // Get the NodeID node and its ping interval, it any.
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(network_status_node,
                                                 ATTRIBUTE_NODE_ID);
  attribute_store_node_t ping_interval_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             PING_INTERVAL_ATTRIBUTE,
                                             0);

  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));

  // If the node just recovered or is being deleted, stop pinging
  if ((false == is_node_failing(network_status_node))
      || (change == ATTRIBUTE_DELETED)) {
    attribute_timeout_cancel_callback(node_id_node, &queue_nop_to_failing_node);
    attribute_store_delete_node(ping_interval_node);
    sl_log_debug(LOG_TAG,
                 "NodeID %d is not failing. It will not be monitored.",
                 node_id);
    return;
  }

  // We only montoring AL and FL failing nodes:
  zwave_operating_mode_t operating_mode
    = network_monitor_get_operating_mode(node_id_node);
  if ((operating_mode != OPERATING_MODE_AL)
      && (operating_mode != OPERATING_MODE_FL)) {
    sl_log_debug(LOG_TAG,
                 "NodeID node %d is neither AL nor FL operating mode. "
                 "It will not be monitored.",
                 node_id);
    return;
  }

  // At that point, the node is failing and we want to NOP it regularly.
  // Make sure the ping interval node is valid
  if (ping_interval_node == ATTRIBUTE_STORE_INVALID_NODE) {
    ping_interval_node
      = attribute_store_add_node(PING_INTERVAL_ATTRIBUTE, node_id_node);
  }

  clock_time_t ping_interval = 0;
  attribute_store_get_reported(ping_interval_node,
                               &ping_interval,
                               sizeof(ping_interval));
  if (ping_interval == 0) {
    if (operating_mode == OPERATING_MODE_AL) {
      ping_interval = AL_NODE_PING_TIME_INTERVAL;
    } else {
      ping_interval = FL_NODE_PING_TIME_INTERVAL;
    }
  }

  sl_log_debug(LOG_TAG,
               "Scheduling NOP command in %lu ms for NodeID: %d.",
               ping_interval,
               node_id);
  attribute_store_set_reported(ping_interval_node,
                               &ping_interval,
                               sizeof(ping_interval));

  attribute_timeout_set_callback(node_id_node,
                                 ping_interval,
                                 &queue_nop_to_failing_node);
}

////////////////////////////////////////////////////////////////////////////////
// Shared/Public functions
////////////////////////////////////////////////////////////////////////////////
void failing_node_monitor_init()
{
  sl_log_info(LOG_TAG, "Initialization of the Failing node monitor");

  // Reset our private variables.
  nop_queued = false;

  // Register Attribute Store callbacks
  attribute_store_register_callback_by_type_and_state(&on_network_status_update,
                                                      DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                                      REPORTED_ATTRIBUTE);
}
