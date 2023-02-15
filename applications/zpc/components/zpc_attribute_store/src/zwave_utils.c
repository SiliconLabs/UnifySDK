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
#include "zwave_utils.h"

// Includes from other components
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_controller_utils.h"
#include "zwave_command_class_indices.h"
#include "zwave_helper_macros.h"

// Generic includes
#include <stdio.h>

#include "sl_log.h"
#define LOG_TAG "zwave_utils"

zwave_operating_mode_t zwave_get_operating_mode(zwave_node_id_t node_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  attribute_store_node_t listening_protocol_node
    = attribute_store_get_first_child_by_type(
      node_id_node,
      ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING);

  if (listening_protocol_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return OPERATING_MODE_UNKNOWN;
  }

  uint8_t listening_protocol = 0;
  if (SL_STATUS_OK
      != attribute_store_read_value(listening_protocol_node,
                                    REPORTED_ATTRIBUTE,
                                    &listening_protocol,
                                    sizeof(listening_protocol))) {
    return OPERATING_MODE_UNKNOWN;
  }

  // The listening bit is the only one that is mandatory for AL nodes to set to
  // 1 and NL/FL nodes to set to 0.
  if (listening_protocol & ZWAVE_NODE_INFO_LISTENING_PROTOCOL_LISTENING_MASK) {
    return OPERATING_MODE_AL;
  }

  // If it is not AL, we find out FL nodes by looking at the beam masks, if any
  // of them is set to 1, it is FL, else NL.
  attribute_store_node_t optional_protocol_node
    = attribute_store_get_first_child_by_type(
      node_id_node,
      ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL);

  if (optional_protocol_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return OPERATING_MODE_UNKNOWN;
  }

  uint8_t optional_protocol = 0;
  if (SL_STATUS_OK
      != attribute_store_read_value(optional_protocol_node,
                                    REPORTED_ATTRIBUTE,
                                    &optional_protocol,
                                    sizeof(optional_protocol))) {
    return OPERATING_MODE_UNKNOWN;
  }

  if (optional_protocol
      & ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_SENSOR_1000MS_MASK) {
    return OPERATING_MODE_FL;
  } else if (optional_protocol
             & ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_SENSOR_250MS_MASK) {
    return OPERATING_MODE_FL;
  } else {
    return OPERATING_MODE_NL;
  }
}

zwave_protocol_t zwave_get_inclusion_protocol(zwave_node_id_t node_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  zwave_protocol_t inclusion_protocol = PROTOCOL_UNKNOWN;

  // No need to verify that inclusion_protocol_node is valid or the value is
  // read correctly. if there is an error, we end returning PROTOCOL_UNKNOWN.
  attribute_store_node_t inclusion_protocol_node
    = attribute_store_get_first_child_by_type(
      node_id_node,
      ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL);

  // Select the desired if defined in case we are trying inclusion protocols/keys
  attribute_store_get_desired_else_reported(inclusion_protocol_node,
                                            &inclusion_protocol,
                                            sizeof(zwave_protocol_t));

  return inclusion_protocol;
}

const char *zwave_get_protocol_name(zwave_protocol_t protocol)
{
  switch (protocol) {
    STR_CASE(PROTOCOL_ZWAVE)
    STR_CASE(PROTOCOL_ZWAVE_LONG_RANGE)
    STR_CASE(PROTOCOL_UNKNOWN)
    default:
      sl_log_warning(LOG_TAG, "Unknown protocol value: %d", protocol);
      return "Unknown";
  }
}

sl_status_t zwave_store_inclusion_protocol(zwave_node_id_t node_id,
                                           zwave_protocol_t inclusion_protocol)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  attribute_store_node_t inclusion_protocol_node
    = attribute_store_get_first_child_by_type(
      node_id_node,
      ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL);

  if (inclusion_protocol_node == ATTRIBUTE_STORE_INVALID_NODE) {
    inclusion_protocol_node
      = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                 node_id_node);
  }

  return attribute_store_set_reported(inclusion_protocol_node,
                                      &inclusion_protocol,
                                      sizeof(zwave_protocol_t));
}

// Read the granted keys for a node from the attribute store
sl_status_t zwave_get_node_granted_keys(zwave_node_id_t node_id,
                                        zwave_keyset_t *keyset)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  // Find the granted keys under the NodeID
  attribute_store_node_t granted_keys_node
    = attribute_store_get_first_child_by_type(node_id_node,
                                              ATTRIBUTE_GRANTED_SECURITY_KEYS);

  return attribute_store_get_reported(granted_keys_node,
                                      keyset,
                                      sizeof(zwave_keyset_t));
}

bool zwave_node_supports_command_class(zwave_command_class_t command_class,
                                       zwave_node_id_t node_id,
                                       zwave_endpoint_id_t endpoint_id)
{
  // Find out the UNID of the node
  unid_t unid;
  zwave_unid_from_node_id(node_id, unid);

  // Retrieve the Attribute Store node for the endpoint:
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint_id);

  // Find the Non-Secure NIF under the endpoint:
  attribute_store_node_t non_secure_nif_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE_ZWAVE_NIF);

  uint8_t nif[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  uint8_t nif_length = 0;

  bool command_class_found = false;

  if ((non_secure_nif_node != ATTRIBUTE_STORE_INVALID_NODE)
      && (SL_STATUS_OK
          == attribute_store_get_node_attribute_value(non_secure_nif_node,
                                                      REPORTED_ATTRIBUTE,
                                                      nif,
                                                      &nif_length))) {
    command_class_found
      = is_command_class_in_supported_list(command_class, nif, nif_length);
  }

  if (command_class_found == true) {
    return command_class_found;
  }

  // We are still not lucky, next stop: look at the secure NIF
  attribute_store_node_t secure_nif_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);
  if (secure_nif_node == ATTRIBUTE_STORE_INVALID_NODE) {
    // Here we can abort, we have nowhere else to look
    return false;
  }

  if (SL_STATUS_OK
      == attribute_store_get_node_attribute_value(secure_nif_node,
                                                  REPORTED_ATTRIBUTE,
                                                  nif,
                                                  &nif_length)) {
    command_class_found
      = is_command_class_in_supported_list(command_class, nif, nif_length);
  }
  return command_class_found;
}

zwave_cc_version_t
  zwave_node_get_command_class_version(zwave_command_class_t command_class,
                                       zwave_node_id_t node_id,
                                       zwave_endpoint_id_t endpoint_id)
{
  unid_t node_unid;
  zwave_unid_from_node_id(node_id, node_unid);
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(node_unid, endpoint_id);

  if (endpoint_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return 0;
  }

  attribute_store_node_t version_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ZWAVE_CC_VERSION_ATTRIBUTE(command_class));

  zwave_cc_version_t version = 0;
  attribute_store_get_reported(version_node, &version, sizeof(version));

  return version;
}

bool zwave_security_validation_is_node_s2_capable(zwave_node_id_t node_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  attribute_store_node_t s2_capable_node
    = attribute_store_get_first_child_by_type(node_id_node,
                                              ATTRIBUTE_NODE_IS_S2_CAPABLE);
  return (s2_capable_node != ATTRIBUTE_STORE_INVALID_NODE);
}

sl_status_t
  zwave_security_validation_set_node_as_s2_capable(zwave_node_id_t node_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);
  attribute_store_node_t s2_capable_node
    = attribute_store_get_first_child_by_type(node_id_node,
                                              ATTRIBUTE_NODE_IS_S2_CAPABLE);
  if (s2_capable_node == ATTRIBUTE_STORE_INVALID_NODE) {
    attribute_store_add_node(ATTRIBUTE_NODE_IS_S2_CAPABLE, node_id_node);
  }
  return SL_STATUS_OK;
}

wake_up_interval_t zwave_get_wake_up_interval(zwave_node_id_t node_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  uint32_t endpoint_id_child_index = 0;
  attribute_store_node_t endpoint_id_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             endpoint_id_child_index);

  wake_up_interval_t wake_up_interval = 0;

  while (endpoint_id_node != ATTRIBUTE_STORE_INVALID_NODE
         && wake_up_interval == 0) {
    // Find if there is a Wake Up Setting:
    // Set up the Wake Up Interval
    attribute_store_node_t wake_up_setting_node
      = attribute_store_get_first_child_by_type(
        endpoint_id_node,
        ATTRIBUTE_COMMAND_CLASS_WAKE_UP_SETTING);
    attribute_store_node_t wake_up_interval_node
      = attribute_store_get_first_child_by_type(
        wake_up_setting_node,
        ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL);

    attribute_store_get_reported(wake_up_interval_node,
                                 &wake_up_interval,
                                 sizeof(wake_up_interval));

    // Move to the next Endpoint
    endpoint_id_child_index += 1;
    endpoint_id_node
      = attribute_store_get_node_child_by_type(node_id_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               endpoint_id_child_index);
  }

  return wake_up_interval;
}

bool is_command_in_array(zwave_command_class_t command_class,
                         zwave_command_t command,
                         const uint8_t *command_list,
                         uint8_t command_list_length)
{
  uint8_t i          = 0;
  bool command_found = false;
  while (i < command_list_length) {
    if (command_list[i] >= EXTENDED_COMMAND_CLASS_IDENTIFIER_START) {
      // 2 bytes CC:
      if (i + 2 >= command_list_length) {
        break;
      }
      zwave_command_class_t current_class
        = (command_list[i] << 8) | (command_list[i + 1]);
      zwave_command_t current_command = command_list[i + 2];
      if (command_class == current_class && command == current_command) {
        command_found = true;
        break;
      }
      // Increment i and try again.
      i += 3;

    } else {
      if (i + 1 >= command_list_length) {
        break;
      }
      zwave_command_class_t current_class = command_list[i];
      zwave_command_t current_command     = command_list[i + 1];
      if (command_class == current_class && command == current_command) {
        command_found = true;
        break;
      }
      // Increment i and try again.
      i += 2;
    }
  }
  return command_found;
}

bool endpoint_exists(zwave_node_id_t node_id, zwave_endpoint_id_t endpoint_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  // Find the granted keys under the NodeID
  return (ATTRIBUTE_STORE_INVALID_NODE
          != attribute_store_get_node_child_by_value(node_id_node,
                                                     ATTRIBUTE_ENDPOINT_ID,
                                                     REPORTED_ATTRIBUTE,
                                                     &endpoint_id,
                                                     sizeof(endpoint_id),
                                                     0));
}