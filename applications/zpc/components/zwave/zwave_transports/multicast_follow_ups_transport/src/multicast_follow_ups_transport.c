/******************************************************************************
* # License
* <b>Copyright 2021  Silicon Laboratories Inc. www.silabs.com</b>
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
#include "multicast_follow_ups_transport.h"

// Generic includes
#include <stdbool.h>
#include <string.h>

// Includes from the Z-Wave Controller
#include "zwave_command_class_indices.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_transport.h"
#include "zwave_controller_internal.h"
#include "zwave_tx.h"
#include "zwave_tx_groups.h"

// Z-Wave API
#include "zwapi_protocol_transport.h"
#include "zwapi_protocol_controller.h"

// Unify Includes
#include "sl_log.h"
#define LOG_TAG "multicast_follow_ups_transport"

// Type definition for our Multicast session
typedef struct multicast_follow_up_session_state {
  // User Callback to invoken when transmission is completed
  on_zwave_tx_send_data_complete_t on_send_data_complete;
  // User pointer to use for the invoking the on_send_data_complete function
  void *user;
  // Are we idle or currently transmitting.
  bool transmission_ongoing;
  // Save the Parent Tx session ID to be able to abort
  zwave_tx_session_id_t parent_session_id;
  // Who's our destination? (Multicast Group ID)
  zwave_multicast_group_id_t group_id;
  // Endpoint ID used for the Multicast session
  zwave_endpoint_id_t remote_endpoint_id;
  // Keeping track of which NodeID is next in the Singlecast follow-ups
  zwave_node_id_t last_singlecast_node_id;
  // QoS Priority of the parent frame
  uint32_t qos_priority;
  // Discard timeout of the parent frame
  uint32_t discard_timeout;
  // Encapsulation used for the destination
  zwave_controller_encapsulation_scheme_t encapsulation;
} multicast_follow_up_session_state_t;

// Local variables
static multicast_follow_up_session_state_t state;

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
static void multicast_follow_ups_reset_state()
{
  zwave_tx_unlock_group(state.group_id);
  state.transmission_ongoing    = false;
  state.parent_session_id       = 0;
  state.group_id                = ZWAVE_TX_INVALID_GROUP;
  state.remote_endpoint_id      = 0;
  state.on_send_data_complete   = NULL;
  state.user                    = NULL;
  state.last_singlecast_node_id = 0;
  state.qos_priority            = 0;
  state.discard_timeout         = 0;
  state.encapsulation           = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
}

static void multicast_follow_ups_terminate_current_session(
  uint8_t status, const zwapi_tx_report_t *tx_info, void *user)
{
  if (state.on_send_data_complete != NULL) {
    state.on_send_data_complete(status, tx_info, user);
  }
  multicast_follow_ups_reset_state();
}

///////////////////////////////////////////////////////////////////////////////
// Z-Wave transport callback functions
///////////////////////////////////////////////////////////////////////////////
void on_multicast_message_send_complete(uint8_t status,
                                        const zwapi_tx_report_t *tx_info,
                                        void *user)
{
  (void)user;  // unused
  // A frame has been delivered
  if (state.transmission_ongoing == false) {
    sl_log_debug(LOG_TAG,
                 "Callback from the Z-Wave Tx while no multicast session "
                 "ongoing. Ignoring.");
    return;
  }

  // check if we want to send more follow-ups
  zwave_controller_connection_info_t info = {};
  zwave_tx_options_t options              = {};

  info.remote.is_multicast = false;
  info.remote.endpoint_id  = state.remote_endpoint_id;
  info.encapsulation       = state.encapsulation;

  options.qos_priority       = state.qos_priority;
  options.discard_timeout_ms = state.discard_timeout;
  options.number_of_responses
    = zwave_tx_get_number_of_responses(state.parent_session_id);

  options.transport.parent_session_id       = state.parent_session_id;
  options.transport.group_id                = state.group_id;
  options.transport.valid_parent_session_id = true;

  // If first follow-up, tell the other transports
  if (state.last_singlecast_node_id == 0) {
    options.transport.is_first_follow_up = true;
  }

  zwave_nodemask_t node_list = {};
  memset(node_list, 0, sizeof(zwave_nodemask_t));

  if (SL_STATUS_OK != zwave_tx_get_nodes(node_list, state.group_id)) {
    sl_log_warning(LOG_TAG,
                   "Cannot retrieve Group ID %d node list. "
                   "Aborting singlecast follow-up frames for id=%p",
                   state.group_id,
                   state.parent_session_id);
    multicast_follow_ups_terminate_current_session(TRANSMIT_COMPLETE_FAIL,
                                                   NULL,
                                                   state.user);
    return;
  }

  for (zwave_node_id_t n = state.last_singlecast_node_id;
       n <= ZW_LR_MAX_NODE_ID;
       n++) {
    if (false == ZW_IS_NODE_IN_MASK(n, node_list)) {
      continue;
    }

    // Take this NodeID and make a follow-up. Save it for next follow-up.
    info.remote.node_id           = n;
    state.last_singlecast_node_id = n + 1;

    sl_status_t send_status
      = zwave_tx_send_data(&info,
                           zwave_tx_get_frame_length(state.parent_session_id),
                           zwave_tx_get_frame(state.parent_session_id),
                           &options,
                           &on_multicast_message_send_complete,
                           NULL,
                           NULL);

    if (send_status != SL_STATUS_OK) {
      sl_log_debug(LOG_TAG,
                   "Tx Queue rejected new frame. "
                   "Aborting multicast session frame id=%p.",
                   state.parent_session_id);
      multicast_follow_ups_terminate_current_session(TRANSMIT_COMPLETE_FAIL,
                                                     NULL,
                                                     state.user);
    }
    // There was more to queue, wait for more callbacks
    return;
  }

  // We have nothing left to do, make the user callback:
  sl_log_debug(LOG_TAG,
               "Multicast session completed for frame id=%p.",
               state.parent_session_id);
  multicast_follow_ups_terminate_current_session(status, tx_info, state.user);
}

///////////////////////////////////////////////////////////////////////////////
// Z-Wave Controller transport functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Verifies if we want automatic singlecast follow-up for a frame
 * and schedule queueing these frames automatically if so.
 *
 * @param info                    Connection information for the frame
 * @param data_length             Length of the frame to send
 * @param data                    Points to the payload to send
 * @param tx_options              Transmit options to use.
 * @param on_send_data_complete   Callback function that will be called when
 *                                the send operation has completed
 * @param user                    User pointer passed in argument of the
 *                                on_send_complete callback function
 * @param parent_session_id       Value of the frame in the TX Queue that is
 *                                the parent of this frame. Frames MUST have
 *                                a valid parent
 *
 * @returns
 * - SL_STATUS_OK The transmission request has been accepted and callback will be
 *                    triggered when the operation is completed.
 * - SL_STATUS_NOT_SUPPORTED   If no endpoint encapsulation is to be applied
 * - SL_STATUS_BUSY            If the transmission cannot be done at the moment.
 */
static sl_status_t multicast_follow_ups_send_data(
  const zwave_controller_connection_info_t *info,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_data_complete,
  void *user,
  zwave_tx_session_id_t parent_session_id)
{
  // Accept the session only if it is a multicast frame that also wants to
  // send some follow-ups.
  if ((info->remote.is_multicast == false)
      || (tx_options->send_follow_ups == false)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Are we already sending?
  if (state.transmission_ongoing == true) {
    sl_log_warning(LOG_TAG,
                   "Z-Wave Multicast Follow ups transport busy sending a "
                   "multicast session (id=%p), we cannot accept a new one.",
                   state.parent_session_id);
    return SL_STATUS_BUSY;
  }

  // Okay, showtime! Let's first requeue the multicast message without the
  // send_follow_ups options.
  zwave_tx_options_t new_options                = *tx_options;
  new_options.transport.parent_session_id       = parent_session_id;
  new_options.transport.valid_parent_session_id = true;
  new_options.transport.group_id                = info->remote.multicast_group;

  // Do not ask for sending more follow-ups, we are taking care of it:
  new_options.send_follow_ups = false;

  sl_status_t send_status
    = zwave_tx_send_data(info,
                         data_length,
                         data,
                         &new_options,
                         &on_multicast_message_send_complete,
                         NULL,
                         NULL);

  // Lock the Tx Group, so the NodeList does not change in the middle of the
  // session.
  zwave_tx_lock_group(info->remote.multicast_group);

  state.transmission_ongoing    = true;
  state.on_send_data_complete   = on_send_data_complete;
  state.user                    = user;
  state.parent_session_id       = parent_session_id;
  state.last_singlecast_node_id = 0;
  state.group_id                = info->remote.multicast_group;
  state.remote_endpoint_id      = info->remote.endpoint_id;
  state.qos_priority            = tx_options->qos_priority;
  state.discard_timeout         = tx_options->discard_timeout_ms;
  state.encapsulation           = info->encapsulation;

  // If the Tx Queue could not take our element, abort already.
  if (send_status != SL_STATUS_OK) {
    sl_log_debug(LOG_TAG,
                 "Tx Queue rejected our multicast session new frame. "
                 "Ignoring frame id=%p",
                 parent_session_id);
    multicast_follow_ups_terminate_current_session(TRANSMIT_COMPLETE_FAIL,
                                                   NULL,
                                                   state.user);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

static sl_status_t
  multicast_follow_ups_abort_send_data(const zwave_tx_session_id_t session_id)
{
  // Are we involved in transmitting this element ?
  if (state.transmission_ongoing != true
      || state.parent_session_id != session_id) {
    return SL_STATUS_NOT_FOUND;
  }

  // If yes, just cancel our session
  multicast_follow_ups_terminate_current_session(TRANSMIT_COMPLETE_FAIL,
                                                 NULL,
                                                 state.user);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Shared functions within the component
///////////////////////////////////////////////////////////////////////////////
sl_status_t multicast_follow_ups_transport_init()
{
  // Get our transmission state initialized.
  multicast_follow_ups_reset_state();

  // Register our transport to the Z-Wave Controller Transport
  zwave_controller_transport_t transport = {0};
  transport.priority                     = 6;
  transport.command_class     = 0x00;  // Does not matter, we cannot decapsulate
  transport.version           = 0x00;  // Does not matter, we cannot decapsulate
  transport.on_frame_received = NULL;  // We cannot decapsulate
  transport.send_data         = &multicast_follow_ups_send_data;
  transport.abort_send_data   = &multicast_follow_ups_abort_send_data;

  return zwave_controller_transport_register(&transport);
}
