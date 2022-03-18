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

// Generic includes
#include <stdio.h>

// Macros
#define STR_CASE(x) \
  case x:           \
    return #x;

zwave_operating_mode_t zwave_get_operating_mode(zwave_node_id_t node_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  attribute_store_node_t listening_protocol_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                             0);

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
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                             0);

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
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                             0);

  attribute_store_read_value(inclusion_protocol_node,
                             REPORTED_ATTRIBUTE,
                             &inclusion_protocol,
                             sizeof(zwave_protocol_t));

  return inclusion_protocol;
}

const char *zwave_get_protocol_name(zwave_protocol_t protocol)
{
  static char message[100];
  switch (protocol) {
    STR_CASE(PROTOCOL_ZWAVE)
    STR_CASE(PROTOCOL_ZWAVE_LONG_RANGE)
    STR_CASE(PROTOCOL_UNKNOWN)
    default:
      snprintf(message, sizeof(message), "%d", protocol);
      return message;
  }
}

sl_status_t zwave_store_inclusion_protocol(zwave_node_id_t node_id,
                                           zwave_protocol_t inclusion_protocol)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  attribute_store_node_t inclusion_protocol_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                             0);

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
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                             0);

  // Get the attribute store reported value for granted keys.
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
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE_ZWAVE_NIF,
                                             0);

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
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE_ZWAVE_SECURE_NIF,
                                             0);
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
  zwave_node_get_command_class_version(uint16_t command_class,
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

  attribute_store_node_t version_node = attribute_store_get_node_child_by_type(
    endpoint_node,
    ZWAVE_CC_VERSION_ATTRIBUTE(command_class),
    0);

  zwave_cc_version_t version = 0;
  attribute_store_get_reported(version_node, &version, sizeof(version));

  return version;
}

bool zwave_security_validation_is_node_s2_capable(zwave_node_id_t node_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  attribute_store_node_t s2_capable_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_NODE_IS_S2_CAPABLE,
                                             0);
  return (s2_capable_node != ATTRIBUTE_STORE_INVALID_NODE);
}

sl_status_t
  zwave_security_validation_set_node_as_s2_capable(zwave_node_id_t node_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);
  attribute_store_node_t s2_capable_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_NODE_IS_S2_CAPABLE,
                                             0);
  if (s2_capable_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return attribute_store_add_node(ATTRIBUTE_NODE_IS_S2_CAPABLE, node_id_node);
  }
  return SL_STATUS_OK;
}