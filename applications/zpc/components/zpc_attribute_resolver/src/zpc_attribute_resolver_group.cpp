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
#include "zpc_attribute_resolver_group.h"
#include "zpc_attribute_resolver_callbacks.h"

// Unify Includes
#include "attribute_resolver_rule.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"

// Includes from other components
#include "attribute_store_defined_attribute_types.h"
#include "zwave_controller_utils.h"
#include "zwave_controller_types.h"
#include "zwave_command_class_supervision.h"
#include "zwave_tx.h"
#include "zwave_tx_groups.h"
#include "zwave_tx_scheme_selector.h"
#include "zwave_utils.h"
#include "sl_log.h"

// Generic includes
#include <set>
#include <array>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

constexpr char LOG_TAG[] = "zpc_attribute_resolver_group";

// A define for how large can a payload be so that we multicast.
// We do not want any PHY limitation or even transport service business in a
// multicast session.
constexpr uint16_t MAXIMUM_MULTICAST_PAYLOAD_LENGTH
  = ZWAVE_TX_SAFE_LOWEST_MAX_PAYLOAD;

// Some defines used for Tx QoS
constexpr uint32_t DEFAULT_QOS
  = ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY;
constexpr uint32_t MULTICAST_QOS_BOOST       = 5 * ZWAVE_TX_RECOMMENDED_QOS_GAP;
constexpr uint32_t FIRST_FOLLOW_UP_QOS_BOOST = 2 * ZWAVE_TX_RECOMMENDED_QOS_GAP;
constexpr uint32_t FL_NODE_QOS_BOOST         = 1 * ZWAVE_TX_RECOMMENDED_QOS_GAP;

/**
 * @brief Struct keeping track of a multicast session
 */
typedef struct multicast_session {
  ///< Assigned Z-Wave TX Group ID for the session.
  zwave_multicast_group_id_t group_id;
  ///< Endpoint ID for the multicast / singlecast follow-ups. It must be identical
  ///< accross all NodeIDs receiving the message
  zwave_endpoint_id_t endpoint_id;
  ///< Protocol on which the node is running
  zwave_protocol_t protocol;
  ///< Encapsulation used for the node
  zwave_controller_encapsulation_scheme_t encapsulation;
  ///< Length of the payload to transmit to the node.
  uint16_t payload_length;
  ///< Payload to transmit to the node. It may differ even for identical Node Types
  std::array<uint8_t, ZWAVE_MAX_FRAME_SIZE> payload;
  ///< If Supervision encapsulation is in use for this session.
  bool use_supervision;
  ///< The Attribute Store nodes that compose the Multicast group.
  std::set<attribute_store_node_t> node_list;
} multicast_session_t;

///////////////////////////////////////////////////////////////////////////////
// Private variables
///////////////////////////////////////////////////////////////////////////////
///< List of multicats candidates. It is Attribute Store Nodes that are pending
///< a set resolution.
static std::set<attribute_store_node_t> multicast_candidates;

///< List of multicast pools, where attribute store nodes share all the same details
///< that allows to make multicast transmissions.
static std::vector<multicast_session_t> multicast_pools;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////
static void on_zwave_group_send_data_complete(uint8_t status,
                                              const zwapi_tx_report_t *tx_info,
                                              void *user);

static void on_supervision_group_send_data_complete(
  uint8_t status, const zwapi_tx_report_t *tx_info, void *user);

// static void on_beam_probe_send_data_complete(uint8_t status,
//                                              const zwapi_tx_report_t *tx_info,
//                                              void *user);

static void on_new_set_rule_registered(attribute_store_type_t type);

static void on_settable_attribute_update(attribute_store_node_t node,
                                         attribute_store_change_t change);

///////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Logs the current list of multicast candidates and multicast pools
 */
void zpc_attribute_resolver_group_state_log()
{
  sl_log_debug(LOG_TAG, "Resolver Group State:");

  std::stringstream stream;
  stream << "- Multicast candidates [";
  for (auto it = multicast_candidates.begin(); it != multicast_candidates.end();
       ++it) {
    stream << *it;
    stream << " ";
  }
  stream << "]\n";

  std::string message = stream.str();
  sl_log_debug(LOG_TAG, "%s", message.c_str());
  sl_log_debug(LOG_TAG, "- Multicast pools:");

  for (auto it = multicast_pools.begin(); it != multicast_pools.end(); ++it) {
    stream.str("");
    stream.clear();
    stream << "\t- Nodes: [";

    for (auto node = it->node_list.begin(); node != it->node_list.end();
         ++node) {
      stream << *node;
      stream << " ";
    }
    stream << "]\n";
    message = stream.str();
    sl_log_debug(LOG_TAG, "%s", message.c_str());
  }
}

/**
 * @brief Assigns a node to a multicast pool.
 *
 * Verifies all the associated data for the node and assign it in an existing
 * pool or creates a new pool to send it.
 *
 * @param node  Node to assign a transmission pool
 * @returns SL_STATUS_OK if the node is already in a pool or has been assigned
 * a transmission pool. Returns SL_STATUS_FAIL otherwise.
 */
static sl_status_t assign_multicast_pool(attribute_store_node_t node)
{
  // First verify if that node is part of any ongoing resolution.
  if (true == is_node_in_resolution_list(node)) {
    return SL_STATUS_OK;
  }
  // Do not do anything with the node if it is paused
  if (true == is_node_or_parent_paused(node)) {
    return SL_STATUS_FAIL;
  }
  // Also do not attempt anything if the node needs a get.
  if (false == attribute_store_is_value_defined(node, REPORTED_ATTRIBUTE)) {
    return SL_STATUS_FAIL;
  }

  // If not, we list the parameters for its pool and try to match it
  zwave_endpoint_id_t endpoint_id = 0;
  zwave_node_id_t node_id         = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_zwave_ids_from_node(node,
                                                                &node_id,
                                                                &endpoint_id)) {
    return SL_STATUS_FAIL;
  }

  zwave_protocol_t protocol = zwave_get_inclusion_protocol(node_id);
  zwave_controller_encapsulation_scheme_t encapsulation
    = zwave_tx_scheme_get_node_highest_security_class(node_id);

  uint16_t payload_length                           = 0;
  std::array<uint8_t, ZWAVE_MAX_FRAME_SIZE> payload = {0};
  attribute_resolver_function_t resolution_function
    = attribute_resolver_set_function(attribute_store_get_node_type(node));

  if (resolution_function == nullptr) {
    return SL_STATUS_FAIL;
  }

  // Abort if it is not a 1-frame kind of resolution.
  if (SL_STATUS_OK
      != resolution_function(node, payload.data(), &payload_length)) {
    return SL_STATUS_FAIL;
  }

  bool use_supervision
    = zwave_node_supports_command_class(COMMAND_CLASS_SUPERVISION,
                                        node_id,
                                        endpoint_id);

  // Node details validation. can this node be part of a multicast?
  if (protocol == PROTOCOL_UNKNOWN) {
    // We do not multicast if we are not sure about the protocol.
    return SL_STATUS_FAIL;
  }
  if (payload_length == 0
      || payload_length > MAXIMUM_MULTICAST_PAYLOAD_LENGTH) {
    // If the payload is 0 or too long, same we just hold off.
    // Singlecast will work equally as fine, and no risk of error.
    return SL_STATUS_FAIL;
  }

  // Verify in which pool it fits:
  for (auto it = multicast_pools.begin(); it != multicast_pools.end(); ++it) {
    // Some transmission is ongoing here, so we don't touch it.
    if (zwave_tx_is_group_locked(it->group_id) == true) {
      continue;
    }
    if (it->endpoint_id != endpoint_id) {
      continue;
    }
    if (it->protocol != protocol) {
      continue;
    }
    if (it->encapsulation != encapsulation) {
      continue;
    }
    if (it->payload_length != payload_length) {
      continue;
    }
    if (it->payload != payload) {
      continue;
    }
    if (it->use_supervision != use_supervision) {
      continue;
    }
    // Last check: NodeID has to differ from the other candidates in the pool.
    bool new_node_id = true;
    for (auto existing_node: it->node_list) {
      zwave_node_id_t existing_node_id = 0;
      attribute_store_network_helper_get_node_id_from_node(existing_node,
                                                           &existing_node_id);
      if (existing_node_id == node_id) {
        new_node_id = false;
        break;
      }
    }
    if (new_node_id == false) {
      return SL_STATUS_ALREADY_EXISTS;
    }

    // Yay, we passed all the checks! Add the node to the pool...
    it->node_list.insert(node);
    return SL_STATUS_OK;
  }

  // We got here, it means we have to create a new pool for this node.
  multicast_session_t new_session = {};
  new_session.group_id            = ZWAVE_TX_INVALID_GROUP;
  new_session.endpoint_id         = endpoint_id;
  new_session.protocol            = protocol;
  new_session.encapsulation       = encapsulation;
  new_session.payload_length      = payload_length;
  new_session.payload             = payload;
  new_session.use_supervision     = use_supervision;
  new_session.node_list.insert(node);
  multicast_pools.push_back(new_session);

  return SL_STATUS_OK;
}

/**
 * @brief Functions that removes a node from the ongoing transmission pools
 *
 * Looks around in the transmission pools and removes the completed node if
 * it is in any of the pools.
 *
 * @param completed_node  Node that can be removed from the transmission pool.
 */
static void multicast_pool_remove_node(attribute_store_node_t completed_node)
{
  for (auto it = multicast_pools.begin(); it != multicast_pools.end(); ++it) {
    for (auto node = it->node_list.begin(); node != it->node_list.end();
         ++node) {
      if (*node == completed_node) {
        it->node_list.erase(node);
        break;
      }
    }
    if (it->node_list.empty()) {
      zwave_tx_unlock_group(it->group_id);
      multicast_pools.erase(it);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Sending function
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Sends the singlecast follow-ups for a Z-Wave TX Group ID.
 *
 * @param group_id  The Group ID for which the Singlecast follow-up frames must
 *                  be enqueued.
 */
static void zpc_attribute_resolver_send_singlecast_follow_ups(
  zwave_multicast_group_id_t group_id)
{
  // Enqueue follow-ups for the multicast pool now.
  for (auto it = multicast_pools.begin(); it != multicast_pools.end(); ++it) {
    if (it->group_id != group_id) {
      continue;
    }
    bool is_first_follow_up = true;
    for (auto node = it->node_list.begin(); node != it->node_list.end();
         ++node) {
      zwave_node_id_t node_id = 0;
      if (SL_STATUS_OK
          != attribute_store_network_helper_get_node_id_from_node(*node,
                                                                  &node_id)) {
        continue;
      }

      // Set out to resolve this node, add it in the list of nodes under resolution
      add_node_in_resolution_list(*node, RESOLVER_SET_RULE);

      zwave_controller_connection_info_t info = {};
      zwave_tx_options_t options              = {};

      info.remote.node_id     = node_id;
      info.remote.endpoint_id = it->endpoint_id;
      info.encapsulation      = it->encapsulation;

      options.qos_priority                 = DEFAULT_QOS;
      options.transport.is_first_follow_up = is_first_follow_up;
      options.transport.group_id           = it->group_id;

      if (zwave_get_operating_mode(node_id) == OPERATING_MODE_FL) {
        options.fasttrack = true;
        options.qos_priority += FL_NODE_QOS_BOOST;
      }

      if (is_first_follow_up == true) {
        options.qos_priority += FIRST_FOLLOW_UP_QOS_BOOST;
        is_first_follow_up = false;
      }

      // Enqueue the frame
      intptr_t user                       = *node;
      sl_status_t send_status             = SL_STATUS_OK;
      zwave_tx_session_id_t tx_session_id = nullptr;
      if (it->use_supervision == true) {
        send_status = zwave_command_class_supervision_send_data(
          &info,
          it->payload_length,
          it->payload.data(),
          &options,
          on_supervision_group_send_data_complete,
          reinterpret_cast<void *>(user),
          &tx_session_id);
        if (send_status != SL_STATUS_OK) {
          // Make a Supervision Fail callback if we could not do it. It will
          // force the resolver to resolve the attribute from scratch
          on_supervision_group_send_data_complete(
            SUPERVISION_REPORT_FAIL,
            nullptr,
            reinterpret_cast<void *>(user));
        }
      } else {
        send_status = zwave_tx_send_data(&info,
                                         it->payload_length,
                                         it->payload.data(),
                                         &options,
                                         on_zwave_group_send_data_complete,
                                         reinterpret_cast<void *>(user),
                                         &tx_session_id);
        if (send_status != SL_STATUS_OK) {
          // Make a send fail callback if we could not do it. It will
          // force the resolver to resolve the attribute from scratch
          on_resolver_zwave_send_data_complete(TRANSMIT_COMPLETE_FAIL,
                                               nullptr,
                                               reinterpret_cast<void *>(user));
        }
      }
      if (send_status == SL_STATUS_OK) {
        attribute_resolver_associate_node_with_tx_sessions_id(*node,
                                                              tx_session_id);
      }
    }
  }
}

/**
 * @brief Triggers the transmissions of all unsent multicast pools.
 *
 * @param initiating_node  The node that the Unify Attribute resolver is trying
 *                         to resolve, which has triggered this whole
 *                         group transmission frenzy.
 *
 * @returns SL_STATUS_NOT_FOUND if the multicast transmissions did not end up
 *          triggerin a transmission of the node parameter. SL_STATUS_OK if the
 *          initiating_node is part of the transmissions pool and its
 *          resolution will be pending.
 */
static sl_status_t
  zpc_attribute_resolver_pools_send(attribute_store_node_t initiating_node)
{
  // Keep track of the node that initially triggered this whole pool send session
  sl_status_t node_send_status = SL_STATUS_NOT_FOUND;

  for (auto it = multicast_pools.begin(); it != multicast_pools.end(); ++it) {
    if (zwave_tx_is_group_locked(it->group_id) == true) {
      // Transmission is already ongoing for this group
      continue;
    }
    if (it->node_list.size() < 2) {
      // We created a pool with only 1 node. Dismiss it !
      it->node_list.clear();
      it->group_id = ZWAVE_TX_INVALID_GROUP;
      continue;
    }

    // Sending time ! Prepare the nodemask and get a Group ID for it.
    zwave_nodemask_t node_mask = {};
    for (auto node = it->node_list.begin(); node != it->node_list.end();
         ++node) {
      zwave_node_id_t node_id = 0;
      if (SL_STATUS_OK
          != attribute_store_network_helper_get_node_id_from_node(*node,
                                                                  &node_id)) {
        // Abort the pool if we can't data about one node
        it->group_id = ZWAVE_TX_INVALID_GROUP;
        break;
      }
      ZW_ADD_NODE_TO_MASK(node_id, node_mask);
    }

    if (SL_STATUS_OK != zwave_tx_assign_group(node_mask, &(it->group_id))) {
      it->group_id = ZWAVE_TX_INVALID_GROUP;
      continue;
    }

    zwave_controller_connection_info_t connection_info = {};
    // Prepare the Connection Info data for remote nodes
    connection_info.local.is_multicast = false;
    connection_info.local.node_id      = 0;
    connection_info.local.endpoint_id  = 0;

    connection_info.remote.is_multicast    = true;
    connection_info.remote.multicast_group = it->group_id;
    connection_info.remote.endpoint_id     = it->endpoint_id;
    connection_info.encapsulation          = it->encapsulation;

    zwave_tx_options_t tx_options = {};
    zwave_tx_scheme_get_node_tx_options(DEFAULT_QOS + MULTICAST_QOS_BOOST,
                                        0,
                                        0,
                                        &tx_options);

    sl_status_t send_status = SL_STATUS_OK;
    if (it->use_supervision == true) {
      send_status = zwave_command_class_supervision_send_data(
        &connection_info,
        it->payload_length,
        it->payload.data(),
        &tx_options,
        nullptr,  // No callback for the multicast frame itself
        nullptr,
        nullptr);

    } else {
      send_status = zwave_tx_send_data(
        &connection_info,
        it->payload_length,
        it->payload.data(),
        &tx_options,
        nullptr,  // No callback for the multicast frame itself
        nullptr,
        nullptr);
    }
    if (send_status == SL_STATUS_OK) {
      zwave_tx_lock_group(it->group_id);
      zpc_attribute_resolver_send_singlecast_follow_ups(it->group_id);
    } else {
      it->group_id = ZWAVE_TX_INVALID_GROUP;
    }
    if (it->node_list.count(initiating_node) != 0) {
      node_send_status = send_status;
    }
  }

  return node_send_status;
}

///////////////////////////////////////////////////////////////////////////////
// Functions shared among the component
///////////////////////////////////////////////////////////////////////////////
sl_status_t zpc_attribute_resolver_send_group(attribute_store_node_t node)
{
  // No need to check if the tranmsission in ongoing, the zpc_attribute_resolver
  // already verified the node_handles.

  // Nobody in the candidates means no multicast:
  if (multicast_candidates.count(node) == 0) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Node is in the list, but the list size if less than 2, no multicast either.
  if (multicast_candidates.size() < 2) {
    multicast_candidates.erase(node);
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Okay so we have a bunch of candidates, sort them into pools now and wipe the
  // candidate list
  for (auto it = multicast_candidates.begin(); it != multicast_candidates.end();
       ++it) {
    assign_multicast_pool(*it);
  }
  multicast_candidates.clear();

  // Now go through the created pools and trigger a transmission,
  // if they are multicast worthy!
  sl_status_t send_status = SL_STATUS_NOT_SUPPORTED;
  if (SL_STATUS_OK == zpc_attribute_resolver_pools_send(node)) {
    send_status = SL_STATUS_OK;
  }

  // Erase inactive groups. (Group ID is left to 0)
  multicast_pools.erase(std::remove_if(multicast_pools.begin(),
                                       multicast_pools.end(),
                                       [](multicast_session_t &s) {
                                         return s.group_id
                                                == ZWAVE_TX_INVALID_GROUP;
                                       }),
                        multicast_pools.end());

  return send_status;
}

///////////////////////////////////////////////////////////////////////////////
// Callback functions
///////////////////////////////////////////////////////////////////////////////
static void on_zwave_group_send_data_complete(uint8_t status,
                                              const zwapi_tx_report_t *tx_info,
                                              void *user)
{
  const attribute_store_node_t received_node = reinterpret_cast<intptr_t>(user);
  multicast_pool_remove_node(received_node);
  on_resolver_zwave_send_data_complete(status, tx_info, user);
}

static void on_supervision_group_send_data_complete(
  uint8_t status, const zwapi_tx_report_t *tx_info, void *user)
{
  const attribute_store_node_t received_node = reinterpret_cast<intptr_t>(user);
  multicast_pool_remove_node(received_node);
  on_resolver_zwave_supervision_complete(status, tx_info, user);
}

static void on_new_set_rule_registered(attribute_store_type_t type)
{
  // We want to get notified before hand when a type needs a set, and the
  // resolver will ask us.
  attribute_store_register_callback_by_type_and_state(
    on_settable_attribute_update,
    type,
    DESIRED_ATTRIBUTE);
}

static void on_settable_attribute_update(attribute_store_node_t node,
                                         attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    multicast_candidates.erase(node);
    return;
  }

  // Is it elligible for the multicast candidate list?
  // At that point we do not check if the node is paused, as it may change
  // by the time we try to issue a multicast.

  if (attribute_store_is_value_defined(node, DESIRED_ATTRIBUTE) == true
      && attribute_store_is_value_matched(node) == false) {
    multicast_candidates.insert(node);
  } else {
    multicast_candidates.erase(node);
  }
}

///////////////////////////////////////////////////////////////////////////////
//  Init and teardown functions.
///////////////////////////////////////////////////////////////////////////////
sl_status_t zpc_attribute_resolver_group_init()
{
  // Ask the resolver to tell us about settable types.
  attribute_resolver_register_set_rule_listener(&on_new_set_rule_registered);
  return SL_STATUS_OK;
}

int zpc_attribute_resolver_group_teardown()
{
  multicast_candidates.clear();
  return 0;
}
