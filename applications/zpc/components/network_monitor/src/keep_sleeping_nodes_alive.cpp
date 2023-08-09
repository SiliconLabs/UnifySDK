/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "keep_sleeping_nodes_alive.h"
#include "network_monitor_utils.h"

// Generic includes
#include <set>
#include <vector>
#include <optional>
#include <iterator>
#include <cassert>

// Unify Components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "sl_log.h"
#include "ctimer.h"

#include "unify_dotdot_attribute_store_node_state.h"

// ZPC Components
#include "attribute_store_type_registration.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_utils.h"
#include "zwave_controller_utils.h"
#include "zwave_tx.h"

constexpr int32_t PING_TIMEOUT_SEC        = 4;
constexpr int32_t GIVE_UP_TIMEOUT_SEC     = 15;
constexpr uint32_t NOP_DISCARD_TIMEOUT_MS = 4000;
constexpr const char *LOG_TAG             = "keep_device_awake";

namespace
{
static struct ctimer poll_timer;
static std::set<attribute_store_node_t> keep_alive;

unsigned long boot_time = clock_seconds();

bool just_booted()
{
  return (clock_seconds() - boot_time) < GIVE_UP_TIMEOUT_SEC;
}

}  // namespace

/**
  * @ingroup network_monitor_keep_alive
* @brief business logic which decides to send a NOP to a node present in the the
  keep alive list.
* - a NOP will be send to nodes inactive longer as `PING_TIMEOUT_SEC`
* - Sending NOP's will be aborted when the node was inactive for longer as
    `GIVE_UP_TIMEOUT_SEC`
* @returns false when the give up timeout occurred. returns true otherwise
*/
static bool keep_alive_controller(
  std::set<attribute_store_node_t>::const_iterator node_id_node_it)
{
  const attribute_store_node_t &node_id_node = *node_id_node_it;

  zwave_node_id_t node_id;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(zwave_node_id_t));

  if (!just_booted()
      && network_monitor_is_node_asleep_due_to_inactivity(
        node_id_node,
        GIVE_UP_TIMEOUT_SEC)) {
    sl_log_debug(
      LOG_TAG,
      "have NOT heard anything from %s %d for more than %d seconds. giving up "
      "on keeping the node awake",
      attribute_store_type_get_node_type_name(node_id_node),
      node_id,
      GIVE_UP_TIMEOUT_SEC);
    return false;
  }

  if (network_monitor_is_node_asleep_due_to_inactivity(node_id_node,
                                                       PING_TIMEOUT_SEC)) {
    sl_log_debug(LOG_TAG,
                 "Sending NOP to %s %i",
                 attribute_store_type_get_node_type_name(node_id_node),
                 node_id);

    zwave_send_nop_to_node(node_id,
                           ZWAVE_TX_QOS_RECOMMENDED_TIMING_CRITICAL_PRIORITY,
                           NOP_DISCARD_TIMEOUT_MS,
                           nullptr,
                           nullptr);
  }

  return true;
}

/**
 * @ingroup network_monitor_keep_alive
 * @brief function that is called by the timer when a timeout occurred. it will
 * call the business logic to see if a device qualifies to be kept alive
 */
static void keep_alive_nop_node([[maybe_unused]] void *user)
{
  assert(!keep_alive.empty());

  retain(keep_alive, keep_alive_controller);
  if (!keep_alive.empty()) {
    ctimer_set(&poll_timer,
               PING_TIMEOUT_SEC * CLOCK_SECOND,
               keep_alive_nop_node,
               nullptr);
  }
}

/**
 * @ingroup network_monitor_keep_alive
 * @brief reads the required pieces from the attribute store. such as network
 status and operating mode.
 * @return SL_STATUS_FAIL in the case the node_id node is invalid, otherwise
 returns status of the attribute store node network get_reported call.
 */
static sl_status_t
  get_node_state_and_operating_mode(attribute_store_node_t network_status_node,
                                    NodeStateNetworkStatus *network_status,
                                    attribute_store_node_t *node_id_node,
                                    zwave_operating_mode_t *operating_mode)
{
  *node_id_node
    = attribute_store_get_first_parent_with_type(network_status_node,
                                                 ATTRIBUTE_NODE_ID);

  if (*node_id_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_error(LOG_TAG, "node_id_node is invalid");
    return SL_STATUS_FAIL;
  }

  *operating_mode = network_monitor_get_operating_mode(*node_id_node);

  sl_status_t res
    = attribute_store_get_reported(network_status_node,
                                   network_status,
                                   sizeof(NodeStateNetworkStatus));

  if (res != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "could not read network status of attribute store node %d",
                 network_status_node);
  }
  return res;
}

/**
 * @ingroup network_monitor_keep_alive
 * @brief stops the timer if the keep_alive list is empty.
 */
static void stop_timer_if_needed()
{
  if (keep_alive.empty()) {
    ctimer_stop(&poll_timer);
  }
}

/**
 * @ingroup network_monitor_keep_alive
 * @brief adds the given node to the keep_alive list and make sure its scheduled
 to keep alive
 * @param node_id_node attribute store node of the node_id attribute
 corresponding its Z-Wave device
 */
static void keep_awake(attribute_store_node_t node_id_node)
{
  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));
  sl_log_debug(LOG_TAG,
               "Preventing %s %d from sleeping until its interview "
               "is completed",
               attribute_store_type_get_node_type_name(node_id_node),
               node_id);

  ctimer_set(&poll_timer,
             PING_TIMEOUT_SEC * CLOCK_SECOND,
             keep_alive_nop_node,
             nullptr);
  keep_alive.insert(node_id_node);
}

/**
  * @ingroup network_monitor_keep_alive
* @brief callback triggered when the networkstatus changes of a given node. when
  the change is not equal to updated it will:
* - add the node to the keep alive list, when network_status ==
    ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING && operating_mode == OPERATING_MODE_NL
* - remove the node from the keep alive list, when above test is false
*/
static void
  keep_alive_network_status_update(attribute_store_node_t network_status_node,
                                   attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  NodeStateNetworkStatus network_status;
  attribute_store_node_t node_id_node;
  zwave_operating_mode_t operating_mode;

  if (sl_status_t result
      = get_node_state_and_operating_mode(network_status_node,
                                          &network_status,
                                          &node_id_node,
                                          &operating_mode);
      result != SL_STATUS_OK) {
    return;
  }

  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(zwave_node_id_t));

  sl_log_debug(LOG_TAG,
               "Network Status Update for NodeID %d, operating mode=%d, "
               "network_status=%d",
               node_id,
               operating_mode,
               network_status);

  if ((network_status == ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING)
      && (operating_mode == OPERATING_MODE_NL)) {
    if (true == is_node_or_parent_paused(node_id_node)) {
      attribute_resolver_set_resolution_resumption_listener(node_id_node,
                                                            &keep_awake);
    } else {
      keep_awake(node_id_node);
    }
  } else {
    keep_alive.erase(node_id_node);
  }

  stop_timer_if_needed();
}

/**
  * @ingroup network_monitor_keep_alive
* @brief remove node from the list if it gets deleted
*/
static void on_node_id_deleted(attribute_store_node_t updated_node,
                               attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    keep_alive.erase(updated_node);
  }
}

void initialize_keep_alive_for_sleeping_nodes()
{
  attribute_store_register_callback_by_type_and_state(
    keep_alive_network_status_update,
    DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type(&on_node_id_deleted,
                                            ATTRIBUTE_NODE_ID);
}
