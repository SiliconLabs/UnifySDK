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
#include "zwave_command_class_association_send.h"
#include "zwave_command_class_association_internals.hpp"
#include "zwave_command_class_agi.h"
#include "zwave_command_class_supervision.h"

// Interfaces
#include "ZW_classcmd.h"

// Includes from other components
#include "zwave_controller_utils.h"
#include "zwave_controller_command_class_indices.h"
#include "zwave_tx.h"
#include "zwave_tx_scheme_selector.h"
#include "zwave_network_management.h"

// UIC includes
#include "attribute.hpp"
#include "sl_log.h"

// Generic includes
#include <map>

using namespace attribute_store;
constexpr char LOG_TAG[] = "command_class_association_send";

// Type definitions
using association_session_id_t = uint32_t;

typedef struct association_tx_session {
  uint16_t number_of_transmissions;
  on_group_send_complete_t callback;
  void *user;
  sl_status_t transmission_status;
} association_tx_session_t;

// Private variables
static association_session_id_t next_session_id = 0;
static std::map<association_session_id_t, association_tx_session_t> sessions;

static void on_association_group_send_complete(uint8_t status,
                                               const zwapi_tx_report_t *tx_info,
                                               void *user)
{
  association_session_id_t received_session
    = (association_session_id_t)(intptr_t)user;

  if (sessions.count(received_session) == 0) {
    sl_log_debug(
      LOG_TAG,
      "Received a TX callback for unknown Association sending session (%d)",
      received_session);
    return;
  }

  sessions[received_session].number_of_transmissions -= 1;
  if ((status != TRANSMIT_COMPLETE_OK)
      && (status != TRANSMIT_COMPLETE_VERIFIED)) {
    sessions[received_session].transmission_status |= SL_STATUS_FAIL;
  }

  if (sessions[received_session].number_of_transmissions <= 0) {
    if (sessions[received_session].callback != nullptr) {
      sessions[received_session].callback(
        sessions[received_session].transmission_status,
        sessions[received_session].user);
    }
    sessions.erase(received_session);
  }
}

sl_status_t
  zwave_association_send_to_group(association_group_id_t group_id,
                                  uint16_t frame_size,
                                  const uint8_t *frame,
                                  bool supporting,
                                  const zwave_tx_options_t *tx_options,
                                  zwave_node_id_t excluded_node,
                                  on_group_send_complete_t callback,
                                  void *user)
{
  if (frame_size < 2) {
    return SL_STATUS_FAIL;
  }

  zwave_command_class_t command_class = frame[0];
  zwave_command_t command             = frame[1];
  if (command_class >= EXTENDED_COMMAND_CLASS_IDENTIFIER_START) {
    if (frame_size < 3) {
      return SL_STATUS_FAIL;
    }
    command_class = ((frame[0] << 8) | frame[1]);
    command       = frame[2];
  }
  // First verify if the Command Class / command pair is registered for this group.
  if (false
      == zwave_command_class_agi_is_command_allowed_for_group(group_id,
                                                              command_class,
                                                              command)) {
    sl_log_warning(
      LOG_TAG,
      "Trying to send illegal commands (0x%x / 0x%x) to Group ID %d. Ignoring",
      command_class,
      command,
      group_id);
    return SL_STATUS_FAIL;
  }

  // Now read the association group content
  association_set associations;
  zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();
  try {
    attribute group(get_group_content_node(zpc_node_id, 0, group_id));
    association_bytes bytestream
      = group.get<association_bytes>(REPORTED_ATTRIBUTE);

    // Convert into a set of associations
    get_association_list(bytestream, associations);
  } catch (const std::exception &e) {
    sl_log_warning(LOG_TAG,
                   "Cannot read association Group ID %d content. Aborting "
                   "transmission attempt. Exception: %s",
                   group_id,
                   e.what());
    return SL_STATUS_FAIL;
  }

  // Remove the associations to the excluded NodeID or ourselves
  std::vector<association_t> excluded_destinations;
  for (auto association: associations) {
    if (association.node_id == excluded_node
        || association.node_id == zpc_node_id) {
      excluded_destinations.push_back(association);
    }
  }
  for (auto association: excluded_destinations) {
    associations.erase(association);
  }
  if (associations.size() == 0) {
    return SL_STATUS_NOT_FOUND;
  }

  // Now transmit a payload for all destinations. We don't bother multicasting.
  zwave_controller_connection_info_t connection = {};

  for (auto association: associations) {
    connection.remote.node_id = association.node_id;
    if (association.type == NODE_ID) {
      connection.remote.endpoint_id = 0;
    } else {
      connection.remote.endpoint_id = association.endpoint_id;
    }
    // This function sets the security scheme at the highest of the receiving node.
    zwave_tx_scheme_get_node_connection_info(connection.remote.node_id,
                                             connection.remote.endpoint_id,
                                             &connection);

    if (supporting == true) {
      // If we are supporting, use our highest key
      zwave_keyset_t zpc_keyset;
      zwave_tx_scheme_get_zpc_granted_keys(&zpc_keyset);
      connection.encapsulation
        = zwave_controller_get_highest_encapsulation(zpc_keyset);
    }

    sl_status_t send_status = SL_STATUS_OK;
    intptr_t user_parameter = next_session_id;
    if (tx_options->number_of_responses == 0
        && true
             == zwave_node_supports_command_class(
               COMMAND_CLASS_SUPERVISION,
               connection.remote.node_id,
               connection.remote.endpoint_id)) {
      send_status = zwave_command_class_supervision_send_data(
        &connection,
        frame_size,
        frame,
        tx_options,
        on_association_group_send_complete,
        (void *)user_parameter,
        nullptr);
    } else {
      send_status = zwave_tx_send_data(&connection,
                                       frame_size,
                                       frame,
                                       tx_options,
                                       &on_association_group_send_complete,
                                       (void *)user_parameter,
                                       nullptr);
    }
    if (send_status != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
  }
  // Save the association session data
  association_tx_session_t new_session = {};
  new_session.number_of_transmissions  = (uint8_t)associations.size();
  new_session.callback                 = callback;
  new_session.user                     = user;
  new_session.transmission_status      = SL_STATUS_OK;
  sessions[next_session_id]            = new_session;

  // Increment for next time.
  next_session_id += 1;

  return SL_STATUS_OK;
}