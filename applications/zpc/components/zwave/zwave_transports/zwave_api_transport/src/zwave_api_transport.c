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
#include "zwave_api_transport.h"

// Generic includes
#include <stdbool.h>
#include <string.h>

// Includes from the Z-Wave Controller
#include "zwave_controller_command_class_indices.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_transport.h"
#include "zwave_controller_internal.h"
#include "zwave_tx.h"
#include "zwave_tx_groups.h"

// Z-Wave API
#include "zwapi_protocol_transport.h"
#include "zwapi_protocol_controller.h"

// Contiki
#include "process.h"

// UIC Includes
#include "sl_log.h"
#define LOG_TAG "zwave_api_transport"

// Local variables
static send_data_state_t state;

// Forward declarations
void zwave_api_send_data_callback(uint8_t status, zwapi_tx_report_t *tx_status);
void zwave_api_transport_on_node_info_req_failed(zwave_node_id_t node_id);

/**
 * @brief Name the of \ref contiki process for the Z-Wave API transport.
 */
PROCESS_NAME(zwave_api_transport_process);

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
static void zwave_api_reset_state()
{
  state.transmission_ongoing  = false;
  state.fast_track            = false;
  state.remote_node_id        = INVALID_NODE_ID;
  state.on_send_data_complete = NULL;
  state.user                  = NULL;
  state.awaiting_nif          = false;

  etimer_stop(&state.emergency_timer);
}

static void zwave_api_transport_start_transmission(
  on_zwave_tx_send_data_complete_t callback,
  void *user,
  zwave_tx_session_id_t parent_session_id)
{
  state.transmission_ongoing  = true;
  state.on_send_data_complete = callback;
  state.user                  = user;
  state.parent_session_id     = parent_session_id;

  process_post(&zwave_api_transport_process,
               ZWAVE_API_TRANSPORT_START_TIMER,
               NULL);
}

static void zwave_api_transport_emergency_timeout_step()
{
  sl_log_error(LOG_TAG,
               "Emergency timer fired! Aborting transmission for element %d\n",
               state.parent_session_id);

  if (state.awaiting_nif == true) {
    // We cannot really abort a Request NIF, just give up on waiting for it.
    // Protocol can handle a new send_data while request nif.
    zwave_api_transport_on_node_info_req_failed(state.remote_node_id);
    return;
  }

  // Use the Z-Wave API to abort.
  zwapi_abort_send_data();
}

// Intercepting Protocol specific commands
static sl_status_t zwave_api_transport_intercept_payload(
  const zwave_controller_connection_info_t *info,
  uint16_t data_length,
  const uint8_t *data)
{
  if (info->remote.is_multicast == true) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (data_length < 1) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Intercept NOPs and try to use the Z-Wave API for it
  if (data[COMMAND_CLASS_INDEX] == ZWAVE_NOP_COMMAND_CLASS) {
    uint8_t zwapi_tx_options = TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE
                               | TRANSMIT_OPTION_EXPLORE;
    sl_status_t status = zwapi_send_nop(info->remote.node_id,
                                        zwapi_tx_options,
                                        &zwave_api_send_data_callback);
    return status;
  }

  if (data_length < 2) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Intercept Request NIFs
  if ((data[COMMAND_CLASS_INDEX] == ZWAVE_PROTOCOL_COMMAND_CLASS)
      && (data[COMMAND_INDEX] == ZWAVE_NODE_INFO_REQUEST_COMMAND)) {
    sl_status_t status = zwapi_request_node_info(info->remote.node_id);

    if (status == SL_STATUS_OK) {
      state.awaiting_nif = true;
      return SL_STATUS_OK;
    } else {
      sl_log_warning(LOG_TAG,
                     "Z-Wave API refused our Request Node Info Command. "
                     "Payload will be sent manually.");
      return SL_STATUS_NOT_SUPPORTED;
    }
  }

  return SL_STATUS_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// Z-Wave API callback functions
///////////////////////////////////////////////////////////////////////////////
void zwave_api_send_data_callback(uint8_t status, zwapi_tx_report_t *tx_status)
{
  if (state.transmission_ongoing == false) {
    sl_log_warning(LOG_TAG,
                   "Callback from the Z-Wave API while no transmission is "
                   "ongoing. Ignoring.");
    return;
  }

  // We are done sending a frame. Notify the calling component.
  if (state.on_send_data_complete != NULL) {
    state.on_send_data_complete(status, tx_status, state.user);
  }

  // We want to notify the Z-Wave Controller about transmission status of a frame.
  // This enables the controller to track when a given node is not reachable.
  if (state.remote_node_id != INVALID_NODE_ID) {
    if ((status == TRANSMIT_COMPLETE_OK)
        || (status == TRANSMIT_COMPLETE_VERIFIED)) {
      zwave_controller_on_frame_transmission(true,
                                             tx_status,
                                             state.remote_node_id);
    } else if (state.fast_track == false) {
      // Inform of transmission failures only if we sent without fast track.
      zwave_controller_on_frame_transmission(false,
                                             tx_status,
                                             state.remote_node_id);
    }
  }

  // We are free again to send frames
  zwave_api_reset_state();
}

void zwave_api_send_test_frame_callback(uint8_t status)
{  // Same as for regular send data, exept that we get no TX report.
  zwave_api_send_data_callback(status, NULL);
}

void zwave_api_send_data_multi_callback(uint8_t status)
{
  // Same as for regular send data, exept that we get no TX report.
  zwave_api_send_data_callback(status, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// Z-Wave controller callback functions
///////////////////////////////////////////////////////////////////////////////
void zwave_api_transport_on_node_info_req_failed(zwave_node_id_t node_id)
{
  (void)node_id;
  if (state.awaiting_nif == true) {
    zwave_api_send_data_callback(TRANSMIT_COMPLETE_FAIL, NULL);
  }
}

void zwave_api_transport_on_node_information(zwave_node_id_t node_id,
                                             const zwave_node_info_t *node_info)
{
  (void)node_info;
  if (state.awaiting_nif == true && state.remote_node_id == node_id) {
    zwave_api_send_data_callback(TRANSMIT_COMPLETE_OK, NULL);
  }
}

static const zwave_controller_callbacks_t zwave_api_transport_callbacks
  = {.on_node_information     = &zwave_api_transport_on_node_information,
     .on_node_info_req_failed = &zwave_api_transport_on_node_info_req_failed};

///////////////////////////////////////////////////////////////////////////////
// Z-Wave Controller transport functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Encapsulates with Multi Channel and send a frame to the Z-Wave TX queue.
 *
 * The provided payload will be Multi Channel encapsulated using the endpoint
 * data provided in
 * connection->remote.endpoint_id and connection->local.endpoint_id.
 *
 * Bit addressing can be used by setting the endpoint value directly (1 bit
 * bit addressing and 7 bits endpoint identifier)
 *
 * @param connection       Connection object describing the source and
 *                         destination. If either the source or destination
 *                         endpoints ID are not null, the frame will
 *                         be Multi Channel encapsulated.
 * @param data_length      Length of the frame to send
 * @param data             Points to the payload to send
 * @param tx_options       Transmit options to use.
 * @param on_multi_channel_complete Callback function that will be called when
 *                                  the send operation has completed
 * @param user             User pointer passed in argument of the on_send_complete
 *                         callback function
 * @param parent_session_id Value of the frame in the TX Queue that is the parent
 *                          of this frame. Frames MUST have a valid parent
 *
 * @returns
 * - SL_STATUS_OK The transmission request has been accepted and callback will be
 *                    triggered when the operation is completed.
 * - SL_STATUS_NOT_SUPPORTED   If no endpoint encapsulation is to be applied
 * - SL_STATUS_BUSY            If the transmission cannot be done at the moment.
 */
static sl_status_t zwave_api_send_data(
  const zwave_controller_connection_info_t *info,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_data_complete,
  void *user,
  zwave_tx_session_id_t parent_session_id)
{
  // Check if we can take the frame length.
  if (data_length > ZWAVE_API_MAXIMUM_FRAME_LENGTH) {
    sl_log_error(LOG_TAG,
                 "Z-Wave API frame too large (size=%d) to be send.",
                 data_length);
    return SL_STATUS_WOULD_OVERFLOW;
  }

  // Are we already sending?
  if (state.transmission_ongoing == true) {
    sl_log_error(LOG_TAG,
                 "Z-Wave API transport busy sending a frame, "
                 "we cannot accept a new one.");
    return SL_STATUS_BUSY;
  }

  // Is it something special like a Request NIF, send NOP?
  if (SL_STATUS_OK
      == zwave_api_transport_intercept_payload(info, data_length, data)) {
    // Mark that we are busy transmitting and return
    state.remote_node_id = info->remote.node_id;
    zwave_api_transport_start_transmission(on_send_data_complete,
                                           user,
                                           parent_session_id);
    return SL_STATUS_OK;
  }

  // Initiate the transmission, sending the frame to the Z-Wave API
  uint8_t zwapi_tx_options = 0;

  // Singlecast or broadcast frame
  if (info->remote.is_multicast == false) {
    if (!IS_BROADCAST_NODE_ID(info->remote.node_id)) {
      // Singlecast are sent with Ack Request.
      zwapi_tx_options = TRANSMIT_OPTION_ACK;
      if (tx_options->fasttrack == false) {
        // Route resolution only if not in "fasttrack" mode
        zwapi_tx_options
          |= TRANSMIT_OPTION_AUTO_ROUTE | TRANSMIT_OPTION_EXPLORE;
      }

      // Save the session state
      state.remote_node_id = info->remote.node_id;
      state.fast_track     = tx_options->fasttrack;
    }

    sl_status_t transmit_status;
    if (tx_options->is_test_frame == true) {
      // Do not count transmit success/failures with test frames
      state.remote_node_id = INVALID_NODE_ID;
      transmit_status
        = zwapi_send_test_frame(info->remote.node_id,
                                tx_options->rf_power,
                                &zwave_api_send_test_frame_callback);
    } else {
      transmit_status = zwapi_send_data(info->remote.node_id,
                                        data,
                                        (uint8_t)data_length,
                                        zwapi_tx_options,
                                        &zwave_api_send_data_callback);
    }

    if (SL_STATUS_OK == transmit_status) {
      // Mark that we are busy transmitting and return
      zwave_api_transport_start_transmission(on_send_data_complete,
                                             user,
                                             parent_session_id);
      return SL_STATUS_OK;
    } else {
      sl_log_warning(LOG_TAG,
                     "Z-Wave API zwapi_send_data/zwapi_send_test_frame refused "
                     "frame (parent_id = %p)\n",
                     parent_session_id);
      return SL_STATUS_FAIL;
    }
  }  // End of singlecast frame

  // Multicast frame
  if (info->remote.is_multicast == true) {
    // No Ack, route or explore frames for tx_options.
    // We don't care about fasttrack here.
    // If the tx_options indicate a group ID for a multicast frame
    // (other than the connection_info.remote), it means that it is a multicast
    // frame to be sent as a broadcast
    if (tx_options->group_id != ZWAVE_TX_INVALID_GROUP) {
      zwapi_tx_options |= TRANSMIT_OPTION_MULTICAST_AS_BROADCAST;
    }

    zwave_nodemask_t node_list = {};
    if (SL_STATUS_OK
        != zwave_tx_get_nodes(node_list, info->remote.multicast_group)) {
      sl_log_warning(LOG_TAG,
                     "Z-Wave Tx cannot read Node List for Group ID %d. "
                     "Dropping frame (parent_id = %p) ",
                     info->remote.multicast_group,
                     parent_session_id);
      return SL_STATUS_FAIL;
    }
    // Try to send.
    if (SL_STATUS_OK
        == zwapi_send_data_multi(node_list,
                                 data,
                                 (uint8_t)data_length,
                                 zwapi_tx_options,
                                 &zwave_api_send_data_multi_callback)) {
      // Mark that we are busy transmitting and return
      zwave_api_transport_start_transmission(on_send_data_complete,
                                             user,
                                             parent_session_id);
      return SL_STATUS_OK;
    } else {
      sl_log_warning(
        LOG_TAG,
        "Z-Wave API zwapi_send_data_multi refused frame (parent_id = %p)\n",
        parent_session_id);
      return SL_STATUS_FAIL;
    }
  }  // End of multicast frame

  // If arrive here, something was uncaught (neither muticast nor singlecast??)
  return SL_STATUS_FAIL;
}

static sl_status_t
  zwave_api_abort_send_data(const zwave_tx_session_id_t session_id)
{
  // Are we involved in transmitting this element ?
  if (state.transmission_ongoing != true
      || state.parent_session_id != session_id) {
    return SL_STATUS_NOT_FOUND;
  }

  if (state.awaiting_nif == true) {
    zwave_api_transport_on_node_info_req_failed(state.remote_node_id);
    return SL_STATUS_OK;
  }

  // If yes, then tell the Z-Wave API to abort.
  zwapi_abort_send_data();
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Shared functions within the component
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_api_transport_init()
{
  // Get our transmission state initialized.
  zwave_api_reset_state();

  // Tell the controller to inform us about NIF events
  zwave_controller_register_callbacks(&zwave_api_transport_callbacks);

  // Start the Z-Wave API transport process
  process_start(&zwave_api_transport_process, NULL);

  // Register our transport to the Z-Wave Controller Transport
  zwave_controller_transport_t transport = {0};
  transport.priority                     = 1;
  transport.command_class     = 0x00;  // Does not matter, we cannot decapsulate
  transport.version           = 0x00;  // Does not matter, we cannot decapsulate
  transport.on_frame_received = NULL;  // We cannot decapsulate
  transport.send_data         = &zwave_api_send_data;
  transport.abort_send_data   = &zwave_api_abort_send_data;

  return zwave_controller_transport_register(&transport);
}

////////////////////////////////////////////////////////////////////////////////
// Contiki Process
////////////////////////////////////////////////////////////////////////////////
PROCESS(zwave_api_transport_process, "zwave_api_transport_process");

PROCESS_THREAD(zwave_api_transport_process, ev, data)
{
  PROCESS_BEGIN();
  (void)data;

  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      sl_log_debug(LOG_TAG, "Z-Wave API transport process initialized.\n");
    } else if (ev == PROCESS_EVENT_EXIT) {
      sl_log_debug(LOG_TAG, "Z-Wave API transport process exited.\n");
    } else if (ev == PROCESS_EVENT_EXITED) {
      // Do not do anything with this event, just wait to go down.
    } else if (ev == ZWAVE_API_TRANSPORT_START_TIMER) {
      // Start the emergency timer, if we are still waiting for the
      // Z-Wave API callback.
      if (state.transmission_ongoing == true) {
        etimer_set(&state.emergency_timer,
                   SEND_DATA_EMERGENCY_TIMEOUT * CLOCK_SECOND);
      }
    } else if (ev == PROCESS_EVENT_TIMER) {
      zwave_api_transport_emergency_timeout_step();
    } else {
      sl_log_warning(LOG_TAG,
                     "Unhandled Z-Wave API transport Event: %d. data=%p\n",
                     ev,
                     data);
    }

    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
