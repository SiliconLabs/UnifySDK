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
#include "zwave_command_class_granted_keys_resolver.h"

// Interfaces
#include "attribute_store_defined_attribute_types.h"
#include "zwave_keyset_definitions.h"
#include "zwave_node_id_definitions.h"
#include "zwave_helper_macros.h"

// ZPC components
#include "zwave_utils.h"
#include "zpc_attribute_resolver.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_tx.h"
#include "zwave_tx_scheme_selector.h"

// Includes from Unify Components
#include "attribute_store.h"
#include "attribute_timeouts.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"

#include "sl_log.h"
#define LOG_TAG "zwave_granted_keys_resolver"

// Constants frames that we will be sending
const uint8_t s2_probe[]
  = {COMMAND_CLASS_SECURITY_2, SECURITY_2_COMMANDS_SUPPORTED_GET};

const uint8_t s0_probe[]
  = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_GET};

/**
 * @brief Remove some discovery combinations if we know the protocol
 *
 * Combinations with a different protocols will be removed.
 *
 * @param node_id_node      Attribute Store node for the NodeId
 * @param protocol          Z-Wave Protocol that we want too keep.
 */
static void exclude_combinations_with_protocol_from_discovery(
  attribute_store_node_t node_id_node, zwave_protocol_t protocol)
{
  attribute_store_node_t endpoint_0_node
    = attribute_store_get_endpoint_0_node(node_id_node);

  uint32_t child_index = 0;
  attribute_store_node_t combination_node
    = attribute_store_get_node_child_by_type(
      endpoint_0_node,
      ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
      child_index);

  while (combination_node != ATTRIBUTE_STORE_INVALID_NODE) {
    zwave_key_protocol_combination_t combination = {};
    attribute_store_get_desired(combination_node,
                                &combination,
                                sizeof(combination));

    if (combination.protocol != protocol) {
      attribute_store_delete_node(combination_node);
      // Restart the iteration if we just deleted one child.
      child_index = 0;
    } else {
      //Get the next node.
      child_index += 1;
    }

    combination_node = attribute_store_get_node_child_by_type(
      endpoint_0_node,
      ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
      child_index);
  }
}

/**
 * @brief Verifies if the Non-secure NIF is already resolved.
 *
 * @param node_id_node      Attribute Store for the NodeID.
 * @returns if the NodeID's NIF is known and resolved, false otherwise.
 */
static bool is_nif_resolved(attribute_store_node_t node_id_node)
{
  attribute_store_node_t endpoint_0_node
    = attribute_store_get_endpoint_0_node(node_id_node);

  attribute_store_node_t nif_node
    = attribute_store_get_first_child_by_type(endpoint_0_node,
                                              ATTRIBUTE_ZWAVE_NIF);

  return attribute_store_is_value_defined(nif_node, REPORTED_ATTRIBUTE);
}

/**
 * @brief Create a list of keys/protocol objects to resolve
 *
 * @param node_id_node      Attribute Store Node for the NodeID.
 * @param s2_support        flag indicating if S2 is in the NIF
 * @param s0_support        flag indicating if S0 is in the NIF
 */
static void create_list_of_keys_protocol_to_resolve(
  attribute_store_node_t node_id_node, bool s2_support, bool s0_support)
{
  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));
  sl_log_info(LOG_TAG,
              "Initiating Granted keys / inclusion protocol "
              "discovery procedure for NodeID %d",
              node_id);
  attribute_store_node_t endpoint_0_node
    = attribute_store_get_endpoint_0_node(node_id_node);

  zwave_key_protocol_combination_t combination = {};
  if (s2_support) {
    combination.key      = ZWAVE_CONTROLLER_S2_ACCESS_KEY;
    combination.protocol = PROTOCOL_ZWAVE;
    attribute_store_emplace_desired(
      endpoint_0_node,
      ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
      &combination,
      sizeof(combination));

    combination.key      = ZWAVE_CONTROLLER_S2_ACCESS_KEY;
    combination.protocol = PROTOCOL_ZWAVE_LONG_RANGE;
    attribute_store_emplace_desired(
      endpoint_0_node,
      ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
      &combination,
      sizeof(combination));

    combination.key      = ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY;
    combination.protocol = PROTOCOL_ZWAVE;
    attribute_store_emplace_desired(
      endpoint_0_node,
      ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
      &combination,
      sizeof(combination));

    combination.key      = ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY;
    combination.protocol = PROTOCOL_ZWAVE_LONG_RANGE;
    attribute_store_emplace_desired(
      endpoint_0_node,
      ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
      &combination,
      sizeof(combination));

    combination.key      = ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY;
    combination.protocol = PROTOCOL_ZWAVE;
    attribute_store_emplace_desired(
      endpoint_0_node,
      ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
      &combination,
      sizeof(combination));
  }
  if (s0_support) {
    combination.key      = ZWAVE_CONTROLLER_S0_KEY;
    combination.protocol = PROTOCOL_ZWAVE;
    attribute_store_emplace_desired(
      endpoint_0_node,
      ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
      &combination,
      sizeof(combination));
  }
  zwave_keyset_t keys = 0;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                     &keys,
                                     sizeof(keys));
}

void on_zwave_key_probe_send_data_complete(uint8_t status,
                                           const zwapi_tx_report_t *tx_info,
                                           void *user)
{
  attribute_store_node_t key_protocol_node
    = (attribute_store_node_t)(intptr_t)user;

  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(key_protocol_node,
                                                 ATTRIBUTE_NODE_ID);

  if (false == IS_TRANSMISSION_SUCCESSFUL(status)) {
    sl_log_debug(LOG_TAG,
                 "Transmission failed for discovering key/protocol. "
                 "Assuming it is not granted.");
    attribute_store_node_t protocol_node
      = attribute_store_get_first_child_by_type(
        node_id_node,
        ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL);
    attribute_store_undefine_desired(protocol_node);
  }

  // Time to wait before restarting probing
  clock_time_t timeout = 0;
  if (tx_info != NULL) {
    timeout += 1000 + tx_info->transmit_ticks * 10;
  }
  attribute_timeout_set_callback(node_id_node,
                                 timeout,
                                 &attribute_resolver_resume_node_resolution);

  attribute_store_delete_node(key_protocol_node);
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Checks if the inclusion protocol just got updated to UNKNOWN, and
 * undefine it if that's the case (because we know how to resolve it now)
 *
 * @param protocol_node       Attribute Store Node for the inclusion protocol
 * @param change              Attribute Store change applied to the node.
 */
static void on_inclusion_protocol_update(attribute_store_node_t protocol_node,
                                         attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  zwave_protocol_t protocol = PROTOCOL_ZWAVE;
  attribute_store_get_reported(protocol_node, &protocol, sizeof(protocol));
  if (protocol == PROTOCOL_UNKNOWN) {
    attribute_store_undefine_reported(protocol_node);
  }
}

/**
 * @brief Checks if the granted keys were just set to 0. In this case,
 * we automatically adjust the Inclusion Protocol to be Z-Wave
 *
 * @param granted_keys_node   Attribute Store Node for the granted keys
 * @param change              Attribute Store change applied to the node.
 */
static void on_granted_keys_update(attribute_store_node_t granted_keys_node,
                                   attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }
  if (false
      == attribute_store_is_value_defined(granted_keys_node,
                                          REPORTED_ATTRIBUTE)) {
    return;
  }

  zwave_keyset_t keys = 0xFF;
  attribute_store_get_reported(granted_keys_node, &keys, sizeof(keys));

  if (keys == 0) {
    attribute_store_node_t node_id_node
      = attribute_store_get_first_parent_with_type(granted_keys_node,
                                                   ATTRIBUTE_NODE_ID);
    zwave_node_id_t node_id = 0;
    attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));
    sl_log_debug(LOG_TAG,
                 "No granted keys for NodeID %d. "
                 "Concluding the Inclusion protocol to be Z-Wave",
                 node_id);
    const zwave_protocol_t protocol = PROTOCOL_ZWAVE;
    attribute_store_set_child_reported(node_id_node,
                                       ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                       &protocol,
                                       sizeof(protocol));
  }
}

/**
 * @brief Reacts to Node Info Frame updates and verify that we will search
 * for granted keys/inclusion protocol next.
 *
 * @param node        Attribute Store Node that is updated
 * @param change      Attribute Store changed applied to the node.
 */
static void on_node_info_update(attribute_store_node_t node,
                                attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_NODE_ID);
  attribute_store_node_t granted_keys_node
    = attribute_store_get_first_child_by_type(node_id_node,
                                              ATTRIBUTE_GRANTED_SECURITY_KEYS);
  attribute_store_node_t inclusion_protocol_node
    = attribute_store_get_first_child_by_type(
      node_id_node,
      ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL);

  attribute_resolver_restart_get_resolution(granted_keys_node);
  attribute_resolver_restart_get_resolution(inclusion_protocol_node);
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_get_granted_keys(
  attribute_store_node_t keyset_node, uint8_t *frame, uint16_t *frame_length)
{
  // This function does not generate any frame.
  *frame_length = 0;

  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(keyset_node,
                                                 ATTRIBUTE_NODE_ID);
  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));

  // First, check if the NIF is resolved. Else just resolve it first.
  if (false == is_nif_resolved(node_id_node)) {
    return SL_STATUS_IS_WAITING;
  }

  // 2. If neither S0 or S2 are supported, conclude non-secure already!
  bool s0_support
    = zwave_node_supports_command_class(COMMAND_CLASS_SECURITY, node_id, 0);
  bool s2_support
    = zwave_node_supports_command_class(COMMAND_CLASS_SECURITY_2, node_id, 0);

  if (false == s0_support && false == s2_support) {
    sl_log_debug(LOG_TAG,
                 "NodeID %d does not support any security, "
                 "assuming it has no Granted Keys.",
                 node_id);
    zwave_keyset_t keyset = 0;
    attribute_store_set_reported(keyset_node, &keyset, sizeof(keyset));
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // See the list of key/protocol combinations to try, then create an attribute
  // under endpoint 0 for this combination
  create_list_of_keys_protocol_to_resolve(node_id_node, s2_support, s0_support);
  return SL_STATUS_IS_WAITING;
}

static sl_status_t zwave_command_class_get_inclusion_protocol(
  attribute_store_node_t protocol_node, uint8_t *frame, uint16_t *frame_length)
{
  // This function does not generate any frame.
  *frame_length = 0;

  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(protocol_node,
                                                 ATTRIBUTE_NODE_ID);
  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));

  // First, check if the NIF is resolved. Else just resolve it first.
  if (false == is_nif_resolved(node_id_node)) {
    return SL_STATUS_IS_WAITING;
  }

  // 2. If neither S0 or S2 are supported, conclude it runs on Z-Wave.
  bool s0_support
    = zwave_node_supports_command_class(COMMAND_CLASS_SECURITY, node_id, 0);
  bool s2_support
    = zwave_node_supports_command_class(COMMAND_CLASS_SECURITY_2, node_id, 0);

  if (false == s0_support && false == s2_support) {
    sl_log_debug(LOG_TAG,
                 "NodeID %d does not support any security, assuming it "
                 "was included with Z-Wave (and not Z-Wave Long Range).",
                 node_id);
    zwave_protocol_t protocol = PROTOCOL_ZWAVE;
    attribute_store_set_reported(protocol_node, &protocol, sizeof(protocol));
    return SL_STATUS_ALREADY_EXISTS;
  }

  // See the list of key/protocol combinations to try, then create an attribute
  // under endpoint 0 for this combination
  create_list_of_keys_protocol_to_resolve(node_id_node, s2_support, s0_support);
  return SL_STATUS_IS_WAITING;
}

static sl_status_t zwave_command_class_probe_key_and_protocol(
  attribute_store_node_t key_protocol_node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  // Look at what we want to probe.
  zwave_key_protocol_combination_t combination = {};
  attribute_store_get_desired(key_protocol_node,
                              &combination,
                              sizeof(combination));

  // Pretend that the node is running this scheme/protocol.
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(key_protocol_node,
                                                 ATTRIBUTE_NODE_ID);

  attribute_store_node_t inclusion_protocol_node
    = attribute_store_get_first_child_by_type(
      node_id_node,
      ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL);
  attribute_store_set_desired(inclusion_protocol_node,
                              &combination.protocol,
                              sizeof(combination.protocol));

  // Just queue the frame directly oursevles.
  zwave_controller_connection_info_t connection = {};
  attribute_store_get_reported(node_id_node,
                               &connection.remote.node_id,
                               sizeof(connection.remote.node_id));
  connection.encapsulation
    = zwave_controller_get_highest_encapsulation(combination.key);

  // Prepare the Z-Wave TX options.
  zwave_tx_options_t tx_options = {0};
  zwave_tx_scheme_get_node_tx_options(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY - ZWAVE_TX_RECOMMENDED_QOS_GAP,
    1,  // We expect 1 answer
    0,
    &tx_options);

  // Give the probe frame to the resolver.
  if (combination.key == ZWAVE_CONTROLLER_S0_KEY) {
    frame[0] = COMMAND_CLASS_SECURITY;
    frame[1] = SECURITY_COMMANDS_SUPPORTED_GET;
  } else {
    frame[0] = COMMAND_CLASS_SECURITY_2;
    frame[1] = SECURITY_2_COMMANDS_SUPPORTED_GET;
  }
  *frame_length = 2;

  const uint8_t *tx_frame = s2_probe;
  if (combination.key == ZWAVE_CONTROLLER_S0_KEY) {
    tx_frame = s0_probe;
  }

  // Queue a maximum priority frame before the resolver.
  sl_status_t transmit_status
    = zwave_tx_send_data(&connection,
                         sizeof(s2_probe),
                         tx_frame,
                         &tx_options,
                         &on_zwave_key_probe_send_data_complete,
                         (void *)(intptr_t)key_protocol_node,
                         NULL);

  if (transmit_status != SL_STATUS_OK) {
    on_zwave_key_probe_send_data_complete(TRANSMIT_COMPLETE_FAIL,
                                          NULL,
                                          (void *)(intptr_t)key_protocol_node);
    return SL_STATUS_ALREADY_EXISTS;
  } else {
    attribute_resolver_pause_node_resolution(node_id_node);
  }

  // Pause the node resolution:
  return SL_STATUS_IS_WAITING;
}

///////////////////////////////////////////////////////////////////////////////
// Functions shared within the component
///////////////////////////////////////////////////////////////////////////////
void zwave_command_class_mark_key_protocol_as_supported(
  attribute_store_node_t node_id_node,
  zwave_controller_encapsulation_scheme_t encapsulation)
{
  attribute_store_node_t granted_keys_node
    = attribute_store_get_first_child_by_type(node_id_node,
                                              ATTRIBUTE_GRANTED_SECURITY_KEYS);
  attribute_store_node_t inclusion_protocol_node
    = attribute_store_get_first_child_by_type(
      node_id_node,
      ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL);

  // Make a bitwise or operation with for the existing keyset.
  zwave_keyset_t used_key
    = zwave_controller_get_key_from_encapsulation(encapsulation);

  zwave_keyset_t reported_keyset = 0;
  attribute_store_get_reported(granted_keys_node,
                               &reported_keyset,
                               sizeof(reported_keyset));

  reported_keyset |= used_key;
  attribute_store_set_reported(granted_keys_node,
                               &reported_keyset,
                               sizeof(reported_keyset));

  // Just copy the desired protocol into the reported protocol.
  if (attribute_store_is_value_defined(inclusion_protocol_node,
                                       DESIRED_ATTRIBUTE)) {
    attribute_store_set_reported_as_desired(inclusion_protocol_node);
    attribute_store_undefine_desired(inclusion_protocol_node);
    zwave_protocol_t protocol = PROTOCOL_UNKNOWN;
    attribute_store_get_reported(inclusion_protocol_node,
                                 &protocol,
                                 sizeof(protocol));
    exclude_combinations_with_protocol_from_discovery(node_id_node, protocol);
  }
}

sl_status_t zwave_command_class_granted_keys_resolver_init()
{
  attribute_resolver_register_rule(ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                   NULL,
                                   &zwave_command_class_get_granted_keys);

  attribute_resolver_register_rule(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                   NULL,
                                   &zwave_command_class_get_inclusion_protocol);

  attribute_resolver_register_rule(ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
                                   NULL,
                                   &zwave_command_class_probe_key_and_protocol);

  attribute_store_register_callback_by_type_and_state(
    &on_granted_keys_update,
    ATTRIBUTE_GRANTED_SECURITY_KEYS,
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    &on_inclusion_protocol_update,
    ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(&on_node_info_update,
                                                      ATTRIBUTE_ZWAVE_NIF,
                                                      REPORTED_ATTRIBUTE);

  return SL_STATUS_OK;
}
