/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zwave_command_handler.h"
#include "zwave_command_handler_internals.h"
#include "zwave_command_handler_callbacks.h"

// Includes from other components
#include "zwave_controller.h"
#include "zwave_controller_keyset.h"
#include "zwave_rx.h"
#include "ZW_classcmd.h"
#include "sl_log.h"

// Generic includes
#include <vector>
#include <algorithm>  // std::find

/// Setup Log ID
#define LOG_TAG "zwave_command_handler_callbacks"

std::set<uint16_t> secure_supported_command_classes;
std::set<uint16_t> nonsecure_supported_command_classes;

// Array buffers that we pass on to the Z-Wave API / S2.
// They keep the uint16_t CC identifiers converted to uint8_t
// when we need to set our NIFs
static std::vector<uint8_t> non_secure_command_class_buffer;
static std::vector<uint8_t> secure_command_class_buffer;

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
static void
  command_class_list_to_buffer(const std::set<uint16_t> &source_list,
                               std::vector<uint8_t> &destination_buffer)
{
  // Clear our static byte array before copying
  destination_buffer.clear();

  // Special treatment for Z-Wave Plus Info CC, it comes first:
  auto it = std::find(source_list.begin(),
                      source_list.end(),
                      COMMAND_CLASS_ZWAVEPLUS_INFO);
  if (it != source_list.end()) {
    destination_buffer.push_back(COMMAND_CLASS_ZWAVEPLUS_INFO);
  }

  // Now copy the other Command Classes one by one.
  for (auto cc: source_list) {
    if (destination_buffer.size() >= ZWAVE_MAX_FRAME_SIZE) {
      return;
    }
    if (cc == COMMAND_CLASS_ZWAVEPLUS_INFO) {
      continue;
    }

    if (cc <= 0xF1) {
      destination_buffer.push_back(cc);
    } else {
      // It's an extended CC, we copy MSB then LSB
      destination_buffer.push_back(cc >> 8);
      destination_buffer.push_back(cc & 0xFF);
    }
  }
}

static void init_command_class_lists()
{
  secure_supported_command_classes.clear();
  nonsecure_supported_command_classes.clear();
  /// FIXME: These guys should probably register themselves too.
  nonsecure_supported_command_classes.insert(COMMAND_CLASS_TRANSPORT_SERVICE);
}

///////////////////////////////////////////////////////////////////////////////
// Callback functions
///////////////////////////////////////////////////////////////////////////////
void zwave_command_handler_on_new_network_entered(
  zwave_home_id_t home_id,
  zwave_node_id_t node_id,
  zwave_keyset_t granted_keys,
  zwave_kex_fail_type_t kex_fail_type)
{
  sl_log_debug(LOG_TAG,
               "Setting NIF for HomeID: %08X NodeID: %03u\n",
               home_id,
               node_id);

  // Reset our CC lists, before we fill them up again
  init_command_class_lists();

  /// Add Security 0 only if ZPC has the S0 key
  if (granted_keys & ZWAVE_CONTROLLER_S0_KEY) {
    nonsecure_supported_command_classes.insert(COMMAND_CLASS_SECURITY);
  }

  /// Update the highest scheme of ZPC that we cache locally
  zpc_highest_scheme = zwave_controller_get_highest_encapsulation(granted_keys);

  /// Add the supported CCs based on the registered command handlers
  zwave_command_handler_it_t it;
  for (it = command_handler_list.begin(); it != command_handler_list.end();
       ++it) {
    if (it->support_handler == nullptr) {
      continue;
    }

    /// TODO: Verify if that works if network scheme is non-secure.
    /// I don't think so.
    if (supports_frame_at_security_level(it,
                                         ZWAVE_CONTROLLER_ENCAPSULATION_NONE)) {
      nonsecure_supported_command_classes.insert(it->command_class);
    } else {
      secure_supported_command_classes.insert(it->command_class);
    }
  }

  /// Set non-secure NIF
  command_class_list_to_buffer(nonsecure_supported_command_classes,
                               non_secure_command_class_buffer);
  zwave_controller_set_application_nif(
    (const uint8_t *)&non_secure_command_class_buffer[0],
    non_secure_command_class_buffer.size());

  /// Set secure NIF
  command_class_list_to_buffer(secure_supported_command_classes,
                               secure_command_class_buffer);
  zwave_controller_set_secure_application_nif(
    (const uint8_t *)&secure_command_class_buffer[0],
    secure_command_class_buffer.size());
}

void zwave_command_handler_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  ///TODO: Verify frame parsing
  sl_status_t status
    = zwave_command_handler_dispatch(connection_info, frame_data, frame_length);
  switch (status) {
    case SL_STATUS_OK:
      sl_log_debug(LOG_TAG,
                   "Command Class=0x%02X - Command=0x%02X from NodeID "
                   "%03d:%d handled successfully.\n",
                   frame_data[0],
                   frame_data[1],
                   connection_info->remote.node_id,
                   connection_info->remote.endpoint_id);
      return;
      break;

    case SL_STATUS_FAIL:
      sl_log_warning(LOG_TAG,
                     "Command Class=0x%02X - Command=0x%02X from "
                     "NodeID %03d:%d had an error "
                     "during handling.\n",
                     frame_data[0],
                     frame_data[1],
                     connection_info->remote.node_id,
                     connection_info->remote.endpoint_id);
      return;
      break;

    case SL_STATUS_BUSY:
      return;
      break;

    case SL_STATUS_NOT_SUPPORTED:
      sl_log_warning(LOG_TAG,
                     "Command Class=0x%02X - Command=0x%02X from "
                     "NodeID %03d:%d gets rejected because "
                     "it is not supported.\n",
                     frame_data[0],
                     frame_data[1],
                     connection_info->remote.node_id,
                     connection_info->remote.endpoint_id);
      return;
      break;

    default:
      sl_log_warning(LOG_TAG,
                     "Command Class=0x%02X - Command=0x%02X had an unexpected "
                     "return status: 0x%04X\n",
                     frame_data[0],
                     frame_data[1],
                     status);
      return;
      break;
  }
}
