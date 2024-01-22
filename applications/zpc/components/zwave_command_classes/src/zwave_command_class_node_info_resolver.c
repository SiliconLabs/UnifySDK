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
#include "zwave_command_class_node_info_resolver.h"
#include "zwave_command_class_version.h"
#include "zwave_command_class_security_0.h"
#include "zwave_command_class_security_2.h"
#include "zwave_command_class_multi_channel.h"

// Generic includes
#include <assert.h>

// Includes from other components
#include "zwave_controller.h"
#include "zwave_controller_utils.h"
#include "zwave_command_class_indices.h"
#include "attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store_network_helper.h"
#include "zpc_attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "zwave_unid.h"
#include "zwave_security_validation.h"
#include "zwave_utils.h"
#include "zpc_attribute_resolver.h"

#include "sl_log.h"
#define LOG_TAG "zwave_command_class_node_info_resolver"

// Private variables
static zwave_controller_callbacks_t zwave_controller_callbacks = {NULL};

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
static void create_secure_nifs_if_missing(attribute_store_node_t node_id_node)
{
  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));

  if (node_id == 0 || node_id_node == get_zpc_node_id_node()) {
    // Don't do that for ourselves
    return;
  }
  // Verify if it is time to create a Secure NIF, for all endpoints under a NodeID:

  // If S0 or S2 keys have been granted, then we create this secure NIF
  // We don't really care if the S0/S2 CCs are present in the non-secure NIF.
  zwave_keyset_t supporting_node_keys = 0;
  zwave_get_node_granted_keys(node_id, &supporting_node_keys);
  zwave_controller_encapsulation_scheme_t supporting_node_scheme
    = zwave_controller_get_highest_encapsulation(supporting_node_keys);

  if (true
      == zwave_controller_encapsulation_scheme_greater_equal(
        supporting_node_scheme,
        ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0)) {
    // Showtime! They have a security key!
    // Find the NodeID in the attribute store, and verify all endpoints.
    uint32_t current_child = 0;
    attribute_store_node_t endpoint_id_node
      = attribute_store_get_node_child_by_type(node_id_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               current_child);
    current_child += 1;

    const attribute_store_type_t attribute[] = {ATTRIBUTE_ZWAVE_SECURE_NIF};
    while (endpoint_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
      attribute_store_add_if_missing(endpoint_id_node,
                                     attribute,
                                     COUNT_OF(attribute));

      endpoint_id_node
        = attribute_store_get_node_child_by_type(node_id_node,
                                                 ATTRIBUTE_ENDPOINT_ID,
                                                 current_child);
      current_child += 1;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute update listeners
///////////////////////////////////////////////////////////////////////////////
static void on_non_secure_nif_update(attribute_store_node_t updated_node,
                                     attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  // Confirm that we got notified about the right type
  assert(ATTRIBUTE_ZWAVE_NIF == attribute_store_get_node_type(updated_node));

  // Verify if it is time to create a Secure NIF for all endpoints
  if (attribute_store_is_value_defined(updated_node, REPORTED_ATTRIBUTE)
      == true) {
    attribute_store_node_t node_id_node
      = attribute_store_get_first_parent_with_type(updated_node,
                                                   ATTRIBUTE_NODE_ID);

    create_secure_nifs_if_missing(node_id_node);
  }
}

static void on_granted_security_key_update(attribute_store_node_t updated_node,
                                           attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  // Confirm that we got notified about the right type
  assert(ATTRIBUTE_GRANTED_SECURITY_KEYS
         == attribute_store_get_node_type(updated_node));

  // Verify if it is time to create a Secure NIF for all endpoints
  if (attribute_store_is_value_defined(updated_node, REPORTED_ATTRIBUTE)
      == true) {
    attribute_store_node_t node_id_node
      = attribute_store_get_first_parent_with_type(updated_node,
                                                   ATTRIBUTE_NODE_ID);

    create_secure_nifs_if_missing(node_id_node);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t resolve_node_info(attribute_store_node_t node,
                                     uint8_t *frame,
                                     uint16_t *frame_length)
{
  // Confirm that we are resolving a ATTRIBUTE_ZWAVE_NIF
  assert(ATTRIBUTE_ZWAVE_NIF == attribute_store_get_node_type(node));

  // Are we trying to resolve our own NIF?
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_NODE_ID);

  if (get_zpc_node_id_node() == node_id_node) {
    attribute_store_delete_node(node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Find the Endpoint ID for who we want the NIF.
  zwave_endpoint_id_t endpoint_id = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_endpoint_id_from_node(
        node,
        &endpoint_id)) {
    *frame_length = 0;
    return SL_STATUS_FAIL;
  }

  if (endpoint_id == 0) {
    // Root device NIF, we used Request NIF from the Protocol CC.
    request_node_info_frame_t *request_nif = (request_node_info_frame_t *)frame;
    request_nif->command_class             = ZWAVE_PROTOCOL_COMMAND_CLASS;
    request_nif->command                   = ZWAVE_NODE_INFO_REQUEST_COMMAND;
    *frame_length                          = sizeof(request_node_info_frame_t);
    return SL_STATUS_OK;
  } else {
    // We need to ask the endpoint capabilities with Multi Channel
    return zwave_command_class_multi_channel_capability_get(node,
                                                            frame,
                                                            frame_length);
  }
}

static sl_status_t resolve_secure_node_info(attribute_store_node_t node,
                                            uint8_t *frame,
                                            uint16_t *frame_length)
{
  // Confirm that we are resolving a ATTRIBUTE_ZWAVE_SECURE_NIF
  assert(ATTRIBUTE_ZWAVE_SECURE_NIF == attribute_store_get_node_type(node));

  // Here it could be either S0 or S2.
  // Base the decision on granted keys (better than CC support in this case)

  // Find the Z-Wave NodeID (endpoint does not matter here, it's the same command).
  zwave_node_id_t node_id = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_node_id_from_node(node, &node_id)) {
    return SL_STATUS_FAIL;
  }

  zwave_keyset_t supporting_node_keys = 0;
  zwave_get_node_granted_keys(node_id, &supporting_node_keys);

  zwave_controller_encapsulation_scheme_t supporting_node_scheme
    = zwave_controller_get_highest_encapsulation(supporting_node_keys);

  if (true
      == zwave_controller_encapsulation_scheme_greater_equal(
        supporting_node_scheme,
        ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED)) {
    return zwave_command_class_security_2_commands_supported_get(frame,
                                                                 frame_length);

  } else if (supporting_node_scheme
             == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0) {
    return zwave_command_class_security_0_commands_supported_get(frame,
                                                                 frame_length);
  }

  *frame_length = 0;
  return SL_STATUS_FAIL;
}

/**
 * @brief Deletes the endpoint (if it is not ep 0) when we failed resolving a NIF
 *
 * @param nif_node  Attribute Store Node for the NIF
 */
static void on_nif_resolution_failed(attribute_store_node_t nif_node)
{
  sl_log_debug(LOG_TAG,
               "NIF Get Resolution failed for Attribute ID %d",
               nif_node);
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(nif_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_get_reported(endpoint_node,
                               &endpoint_id,
                               sizeof(endpoint_id));

  if (endpoint_id != 0) {
    sl_log_debug(LOG_TAG,
                 "Deleting Endpoint %d (Attribute ID %d) "
                 "as we cannot resolve its NIF",
                 endpoint_id,
                 endpoint_node);
    attribute_store_delete_node(endpoint_node);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Z-Wave Controller callback functions
///////////////////////////////////////////////////////////////////////////////
static void on_node_information_update(zwave_node_id_t node_id,
                                       const zwave_node_info_t *node_info)
{
  // Find the node from the attribute store:
  unid_t unid;
  zwave_unid_from_node_id(node_id, unid);

  // Find the Non-secure NIF for the node/endpoint 0
  attribute_store_node_t endpoint_id_node
    = attribute_store_network_helper_get_endpoint_node(unid, 0);

  attribute_store_node_t non_secure_nif_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_NIF);

  // Pack the NIF data into a nif array:
  uint8_t nif[ZWAVE_CONTROLLER_MAXIMUM_COMMAND_CLASS_LIST_LENGTH];
  uint8_t nif_length = 0;
  zwave_command_class_list_pack(node_info, nif, &nif_length);

  // Security time
  // Before we trust that input, verify if some Security CC just disappeared
  // from the NIF, when it should not have.
  if (true
      == zwave_security_validation_is_s2_nif_downgrade_attack_detected(
        node_id,
        nif,
        nif_length)) {
    if (nif_length >= ZWAVE_CONTROLLER_MAXIMUM_COMMAND_CLASS_LIST_LENGTH) {
      // Overflow, just return and toss the faulty NIF.
      return;
    }
    // Keep S2 in the NIF !
    nif[nif_length] = COMMAND_CLASS_SECURITY_2;
    nif_length += 1;
  }

  attribute_store_set_reported(non_secure_nif_node, nif, nif_length);

  // We just wrote down a new NIF.
  // Verify if it is the first time that we detect a node supports S2/S0:
  if (zwave_node_supports_command_class(COMMAND_CLASS_SECURITY_2, node_id, 0)
      == true) {
    zwave_security_validation_set_node_as_s2_capable(node_id);
  }

  // Save the additional data: (Protocol listening / Optional protocol)
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(endpoint_id_node,
                                                 ATTRIBUTE_NODE_ID);
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                     &node_info->listening_protocol,
                                     sizeof(node_info->listening_protocol));

  // Find the optional protocol byte from the NIF
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                     &node_info->optional_protocol,
                                     sizeof(node_info->optional_protocol));

  // Save all the device type information.
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE_ZWAVE_BASIC_DEVICE_CLASS,
                                     &node_info->basic_device_class,
                                     sizeof(node_info->basic_device_class));

  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE_ZWAVE_GENERIC_DEVICE_CLASS,
                                     &node_info->generic_device_class,
                                     sizeof(node_info->generic_device_class));

  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE_ZWAVE_SPECIFIC_DEVICE_CLASS,
                                     &node_info->specific_device_class,
                                     sizeof(node_info->specific_device_class));
}

static void on_secure_node_info_no_response(attribute_store_node_t secure_nif_node)
{
  uint8_t nif[ZWAVE_CONTROLLER_MAXIMUM_COMMAND_CLASS_LIST_LENGTH];
  uint8_t nif_length = 0;

  if ((SL_STATUS_OK
       == attribute_store_get_node_attribute_value(secure_nif_node,
						   REPORTED_ATTRIBUTE,
						   nif,
						   &nif_length))
      && (nif_length != 0)) {
      return;
  }

  sl_log_warning(LOG_TAG, "Failed to get Secure NIF Attribute ID %d", secure_nif_node);

  // Are we trying to resolve our own NIF?
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(secure_nif_node, ATTRIBUTE_NODE_ID);

  if (get_zpc_node_id_node() == node_id_node) {
    return;
  }

  // Find the Endpoint ID for who we want the NIF.
  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_node_t endpoint_id_node
    = attribute_store_get_first_parent_with_type(secure_nif_node, ATTRIBUTE_ENDPOINT_ID);
  if (endpoint_id_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return;
  }
  if (SL_STATUS_OK
      != attribute_store_get_reported(endpoint_id_node,
                                      &endpoint_id,
                                      sizeof(zwave_endpoint_id_t))) {
    // Abort if no Endpoint ID data was retrieved.
    return;
  }

  // If Multi Channel endpoint
  if (endpoint_id <= 0) {
    return;
  }
  attribute_store_node_t non_secure_nif_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_NIF);

  if ((non_secure_nif_node != ATTRIBUTE_STORE_INVALID_NODE)
      && (SL_STATUS_OK
	  == attribute_store_get_node_attribute_value(non_secure_nif_node,
						      REPORTED_ATTRIBUTE,
						      nif,
						      &nif_length))) {
    uint8_t j = 0;
    for (uint8_t i=0; i<nif_length; i++) {
      if (nif[i] != COMMAND_CLASS_SECURITY && nif[i] != COMMAND_CLASS_SECURITY_2) {
	nif[j++] = nif[i];
      }
    }
    nif_length = j;
    
    // The implicit rule that all non-secure command classes for an End Point
    // must be controllable securely is still in effect,
    // if the endpoint is reported secure.
    attribute_store_set_reported(secure_nif_node, nif, nif_length);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_node_info_resolver_init()
{
  // Register a callbacks for Node info frames
  zwave_controller_callbacks.on_node_information = on_node_information_update;
  zwave_controller_register_callbacks(&zwave_controller_callbacks);

  // Register rules to resolve NIFs
  attribute_resolver_register_rule(ATTRIBUTE_ZWAVE_NIF,
                                   NULL,
                                   resolve_node_info);

  // Get notified here if we give up trying to resolve a NIF.
  attribute_resolver_set_resolution_give_up_listener(ATTRIBUTE_ZWAVE_NIF,
                                                     &on_nif_resolution_failed);

  attribute_resolver_register_rule(ATTRIBUTE_ZWAVE_SECURE_NIF,
                                   NULL,
                                   resolve_secure_node_info);

  // Listen to Non-secure NIF updates from the attribute store
  attribute_store_register_callback_by_type(&on_non_secure_nif_update,
                                            ATTRIBUTE_ZWAVE_NIF);

  attribute_store_register_callback_by_type(&on_granted_security_key_update,
                                            ATTRIBUTE_GRANTED_SECURITY_KEYS);

  // Attribute Resolver give up listener:
  attribute_resolver_set_resolution_give_up_listener(
    ATTRIBUTE_ZWAVE_SECURE_NIF,
    &on_secure_node_info_no_response);

  return SL_STATUS_OK;
}
