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

//Generic includes
#include <stdlib.h>
#include <stdio.h>

// Includes from other components
#include "sl_log.h"
#include "zpc_converters.h"
#include "zwave_unid.h"
#include "zwave_tx_scheme_selector.h"

// Includes from this component
#include "zwave_controller_utils.h"
#include "zwave_controller_types.h"
#include "zwave_controller_keyset.h"
#include "zwave_controller_command_class_indices.h"

// Setup Log ID
#define LOG_TAG "zwave_controller_utils"

void zwave_send_nop_to_node(zwave_node_id_t node,
                     const on_zwave_tx_send_data_complete_t on_send_complete,
                     void *user)
{
  // Prepare the Connection Info data:
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node;
  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;

  // Prepare the Z-Wave TX options.
  zwave_tx_options_t tx_options = {0};
  tx_options.qos_priority       = ZWAVE_TX_QOS_MIN_PRIORITY;

  uint8_t nop_frame[1]    = {0};
  sl_log_debug(LOG_TAG, "Sending NOP to Failing Node: %d.", node);
  zwave_tx_send_data(&connection_info,
                     sizeof(nop_frame),
                     nop_frame,
                     &tx_options,
                     0,
                     on_send_complete,
                     user);
}

void zwave_sl_log_frame_data(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Just make a nice print of the frame data
  char message[1000];
  uint16_t index = 0;

  index += snprintf(message + index,
                    sizeof(message) - index,
                    "Z-Wave Frame (NodeID %d:%d -> %d:%d ",
                    connection_info->remote.node_id,
                    connection_info->remote.endpoint_id,
                    connection_info->local.node_id,
                    connection_info->local.endpoint_id);

  if (connection_info->local.is_multicast) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "via multicast/broadcast.");
  } else {
    index
      += snprintf(message + index, sizeof(message) - index, "via singlecast.");
  }

  index += snprintf(message + index,
                    sizeof(message) - index,
                    " Status flags: 0x%02X - ",
                    rx_options->status_flags);
  index += snprintf(message + index,
                    sizeof(message) - index,
                    "RSSI: %d dBm - Frame payload (hex): ",
                    rx_options->rssi);

  for (uint8_t i = 0; i < frame_length; i++) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "%02X ",
                      frame_data[i]);
  }
  sl_log_debug(LOG_TAG, "%s\n", message);
}

void zwave_sl_log_nif_data(zwave_node_id_t node_id,
                           const zwave_node_info_t *node_info)
{
  // Just make a nice print of the NIF data
  char message[1000];
  uint16_t index = 0;

  index += snprintf(message + index,
                    sizeof(message) - index,
                    "NIF from NodeID: %d",
                    node_id);

  index += snprintf(message + index,
                    sizeof(message) - index,
                    " Capability/Security bytes: 0x%02X 0x%02X - ",
                    node_info->listening_protocol,
                    node_info->optional_protocol);

  if (node_info->optional_protocol
      & ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_CONTROLLER_MASK) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "The node is a controller - ");
  } else {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "The node is an end node - ");
  }

  if (node_info->listening_protocol
      & ZWAVE_NODE_INFO_LISTENING_PROTOCOL_LISTENING_MASK) {
    index += snprintf(message + index, sizeof(message) - index, "AL mode - ");
  } else if (node_info->optional_protocol
             & (ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_SENSOR_1000MS_MASK
                | ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_SENSOR_250MS_MASK)) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "FL mode (FLiRS) - ");
  } else {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "NL mode (Non-Listening) - ");
  }

  index += snprintf(message + index,
                    sizeof(message) - index,
                    "Basic, Generic and Specific Device Classes: 0x%02X 0x%02X "
                    "0x%02X - Supported CC list: ",
                    node_info->basic_device_class,
                    node_info->generic_device_class,
                    node_info->specific_device_class);

  for (uint8_t i = 0; i < node_info->command_class_list_length; i++) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "%02X ",
                      node_info->command_class_list[i]);
  }

  sl_log_debug(LOG_TAG, "%s", message);
}

void zwave_sl_log_dsk(const char *tag, const zwave_dsk_t dsk)
{
  (void)tag;  // Unused in Z-Wave build.
  char message[1000];
  uint16_t index = 0;
  index += snprintf(message + index, sizeof(message) - index, "DSK: ");
  zpc_converters_dsk_to_str(dsk, message + index, sizeof(message) - index);
  sl_log_info(tag, "%s\n", message);
}

void zwave_command_class_list_unpack(zwave_node_info_t *node_info,
                                     const uint8_t *nif,
                                     uint8_t nif_length)
{
  uint8_t i = 0, nif_index = 0;
  while ((nif_index < nif_length)
         && (i < ZWAVE_CONTROLLER_MAXIMUM_COMMAND_CLASS_LIST_LENGTH)) {
    // Check if it's extended CC format (2 bytes length). Check
    // CC:0000.00.00.11.001 for more details
    if (((nif[nif_index] & 0xF0) == 0xF0) && (nif_index < nif_length - 1)) {
      node_info->command_class_list[i]
        = ((nif[nif_index] & 0xFF) << 8) | nif[nif_index + 1];
      i++;
      nif_index += 2;
    } else {
      node_info->command_class_list[i++] = nif[nif_index++];
    }
    node_info->command_class_list_length = i;
  }
}

void zwave_command_class_list_pack(const zwave_node_info_t *node_info,
                                   uint8_t *nif,
                                   uint8_t *nif_length)
{
  uint8_t nif_index = 0;
  for (uint8_t i = 0; i < node_info->command_class_list_length
                      && i < ZWAVE_CONTROLLER_MAXIMUM_COMMAND_CLASS_LIST_LENGTH;
       i++) {
    if (node_info->command_class_list[i] >= 0xF0) {
      nif[nif_index++] = node_info->command_class_list[i] >> 8;
      nif[nif_index++] = node_info->command_class_list[i];
    } else {
      nif[nif_index++] = node_info->command_class_list[i];
    }
  }
  *nif_length = nif_index;
}

bool is_command_class_in_supported_list(zwave_command_class_t command_class,
                                        const uint8_t *nif,
                                        uint8_t nif_length)
{
  // Look into the contents and look for our command_class
  uint8_t i = 0;
  while (i < nif_length) {
    if (nif[i] == COMMAND_CLASS_CONTROL_MARK) {
      break;
    } else if (nif[i] >= EXTENDED_COMMAND_CLASS_IDENTIFIER_START
               && (i < nif_length - 1)) {
      if (command_class == ((nif[i] << 8) | (nif[i + 1]))) {
        return true;
      }
      i += 2;  //Extra increment of the index, due to 2 bytes CC.
    } else {
      if (command_class == (nif[i])) {
        return true;
      }
      i++;  // Simple increment
    }
  }
  return false;
}

/**
 * @brief Parses a byte array for Command Classes until the end or a
 * COMMAND_CLASS_CONTROL_MARK
 *
 * @param nif               The byte array to parse for Command Class
 *                          identifiers to look for.
 * @param nif_length        The number of elements contained in the nif byte
 *                          array
 * @param final_nif_index   The index at which we stopped parsing the nif byte
 *                          array, due to end of array or a
 *                          COMMAND_CLASS_CONTROL_MARK
 * @param user_list         A pointer to an array for copying the list of parsed
 *                          Command Class identifiers.
 * @param user_list_length  A pointer to write the number of elements that have
 *                          been copied to the user_list array.

 * @returns SL_STATUS_OK if parsing went well.
 */
static sl_status_t
  zwave_parse_nif_list_until_marker_or_end(const uint8_t *nif,
                                           size_t nif_length,
                                           size_t *final_nif_index,
                                           zwave_command_class_t *user_list,
                                           size_t *user_list_length)
{
  // NULL pointer, we do not copy anything and our job is done
  if (user_list == NULL) {
    *final_nif_index = 0;
    return SL_STATUS_OK;
  }

  // Look into the contents and look for our command_class
  uint8_t nif_index = 0;
  size_t user_index = 0;
  while ((nif_index < nif_length) && (nif_index < ZWAVE_MAX_NIF_SIZE)) {
    if (nif[nif_index] == COMMAND_CLASS_CONTROL_MARK) {
      nif_index += 1;
      break;

    } else if (nif[nif_index] >= EXTENDED_COMMAND_CLASS_IDENTIFIER_START) {
      if (nif_index < nif_length - 1) {
        user_list[user_index] = ((nif[nif_index] << 8) | (nif[nif_index + 1]));
        user_index += 1;
        nif_index += 2;  //Extra increment of the index, due to 2 bytes CC.
      } else {
        break;
      }

    } else {
      user_list[user_index] = nif[nif_index];
      nif_index += 1;
      user_index += 1;
    }
  }

  // We are done.
  *final_nif_index  = nif_index;
  *user_list_length = user_index;
  return SL_STATUS_OK;
}

sl_status_t zwave_parse_nif(const uint8_t *nif,
                            size_t nif_length,
                            zwave_command_class_t *supported_command_classes,
                            size_t *supported_command_classes_length,
                            zwave_command_class_t *controlled_command_classes,
                            size_t *controlled_command_classes_length)
{
  // Use the static helper function for each list
  sl_status_t status     = SL_STATUS_OK;
  size_t final_nif_index = 0;

  status |= zwave_parse_nif_list_until_marker_or_end(
    nif,
    nif_length,
    &final_nif_index,
    supported_command_classes,
    supported_command_classes_length);

  status |= zwave_parse_nif_list_until_marker_or_end(
    &nif[final_nif_index],
    nif_length - final_nif_index,
    &final_nif_index,
    controlled_command_classes,
    controlled_command_classes_length);

  return status;
}

const char *
  zwave_network_scheme_str(zwave_controller_encapsulation_scheme_t scheme)
{
  const char *security_level;
  switch (scheme) {
    case ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME:
      security_level = "Network Scheme";
      break;
    case ZWAVE_CONTROLLER_ENCAPSULATION_NONE:
      security_level = "Unencrypted";
      break;
    case ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0:
      security_level = "Security Scheme 0";
      break;
    case ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED:
      security_level = "Security 2, unauthenticated";
      break;
    case ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED:
      security_level = "Security 2, authenticated";
      break;
    case ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS:
      security_level = "Security 2, access";
      break;
    default:
      security_level = "Unknown";
      break;
  }
  return security_level;
}

zwave_controller_encapsulation_scheme_t zpc_highest_security_class()
{
  zwave_keyset_t zpc_keyset;
  zwave_tx_scheme_get_zpc_granted_keys(&zpc_keyset);
  return zwave_controller_get_highest_encapsulation(zpc_keyset);
}
