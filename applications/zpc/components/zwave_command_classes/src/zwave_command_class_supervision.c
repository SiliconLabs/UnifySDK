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
#include "zwave_command_class_supervision.h"
#include "zwave_command_class_supervision_internals.h"
#include "zwave_command_class_supervision_process.h"
#include "zwave_command_class_indices.h"
#include "zwave_command_class_wake_up.h"
#include "zwave_command_classes_utils.h"

// Interface includes
#include "zwave_command_class_wake_up_types.h"

// Includes from other components
#include "sl_log.h"
#include "attribute_store.h"
#include "attribute_resolver.h"
#include "zwave_unid.h"
#include "ZW_classcmd.h"
#include "zwave_tx.h"
#include "zwave_controller_keyset.h"
#include "zwave_controller_utils.h"
#include "zwave_utils.h"
#include "zwave_command_handler.h"

// Generic includes
#include "assert.h"
#include "string.h"

// Log tag
#define LOG_TAG "zwave_command_class_supervision"

// Keep track of Supervision Sessions, when we are controlled.
// We support only 1 session at a time, would abort an ongoing operation
// for a new one if needed.
static supervision_session_t supported_session = {0};

// Keep a list of nodes that must be "awaken" on demand
static zwave_nodemask_t wake_on_demand_list = {0};

///////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_supervision_handle_supervision_get(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect at least 1 bytes of encapsulated command according to the spec.
  // Else it's just a fail.
  if (frame_length <= SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX + 1) {
    return SL_STATUS_FAIL;
  }
  // Duplicate detection for the support of supervision_get command
  // We do not return anything in case of duplicate info in 2 consecutive singlecast commands.
  // CC:006C.01.01.11.00D Do not send a report in case of duplicate if no security encap was applied
  // We tend to accept duplicate SessionID else, due to CL:006C.01.52.01.1
  if (((frame_data[SUPERVISION_GET_SESSION_ID_INDEX]
        & SUPERVISION_GET_PROPERTIES1_SESSION_ID_MASK)
       == supported_session.session_id)
      && (connection_info->remote.node_id == supported_session.node_id)
      && (connection_info->remote.endpoint_id == supported_session.endpoint_id)
      && (connection_info->encapsulation == ZWAVE_CONTROLLER_ENCAPSULATION_NONE)
      && connection_info->local.is_multicast == false) {
    sl_log_debug(LOG_TAG, "Ignoring Supervision Get duplicate");
    return SL_STATUS_OK;
  }

  // If we could not get a status from a command handler, it is
  // recommended to default to NO_SUPPORT. FAIL encourages the
  // controlling node to try again.
  sl_status_t command_handler_status = SL_STATUS_NOT_SUPPORTED;

  // Unwrapping SUPERVISION encapsulation and insert to the Command Handler framework
  uint8_t encapsulated_frame_length
    = frame_data[SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX];
  const uint8_t *unwrapped_frame_data
    = frame_data + (SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX + 1);
  if (encapsulated_frame_length
      <= frame_length
           - (SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX + 1)) {
    // Inject into the command handler only if parsing goes well
    // The same command will be injected again in a singlecast follow-up.
    // Normally we should already be at target value, so it does not harm to
    // do it again.
    command_handler_status
      = zwave_command_handler_dispatch(connection_info,
                                       unwrapped_frame_data,
                                       (uint16_t)encapsulated_frame_length);
  }

  // Return a response if it was the first singlecast.
  if (connection_info->local.is_multicast == false) {
    // Accept the SessionID and copy the data.
    supported_session.session_id
      = frame_data[SUPERVISION_GET_SESSION_ID_INDEX]
        & SUPERVISION_GET_PROPERTIES1_SESSION_ID_MASK;
    supported_session.node_id     = connection_info->remote.node_id;
    supported_session.endpoint_id = connection_info->remote.endpoint_id;
    // Start preparing a report.
    ZW_SUPERVISION_REPORT_FRAME report;
    report.cmdClass    = COMMAND_CLASS_SUPERVISION;
    report.cmd         = SUPERVISION_REPORT;
    report.duration    = 0;
    report.properties1 = supported_session.session_id;

    // More Status Update is not supported as we never return WAITING.
    // Do not add the WAITING return code, unless you implement support for More Status Update.
    switch (command_handler_status) {
      case SL_STATUS_OK:
        report.status = SUPERVISION_REPORT_SUCCESS;
        break;
      case SL_STATUS_FAIL:
      case SL_STATUS_BUSY:
        report.status = SUPERVISION_REPORT_FAIL;
        break;
      case SL_STATUS_NOT_SUPPORTED:
      default:
        report.status = SUPERVISION_REPORT_NO_SUPPORT;
        break;
    }

    // Do we need to Wake Up on Demand?
    // (either via ZPC Stin command or via pending resolution)
    attribute_store_node_t node
      = zwave_command_class_get_node_id_node(connection_info);
    if (zwave_command_class_wake_up_supports_wake_up_on_demand(node) == true) {
      if (true
          == ZW_IS_NODE_IN_MASK(connection_info->remote.node_id,
                                wake_on_demand_list)) {
        report.properties1 |= SUPERVISION_REPORT_PROPERTIES1_WAKE_UP_BIT_MASK;
      } else if (is_node_or_parent_paused(node) == true
                 && attribute_resolver_node_or_child_needs_resolution(node)
                      == true) {
        report.properties1 |= SUPERVISION_REPORT_PROPERTIES1_WAKE_UP_BIT_MASK;
      }
    }

    // Send our Supervision Report.
    sl_status_t send_data_status
      = zwave_command_class_send_report(connection_info,
                                        sizeof(report),
                                        (uint8_t *)&report);

    if (send_data_status == SL_STATUS_OK) {
      // Remove the node from the Wake On Demand list in any case
      zwave_command_class_supervision_stop_wake_on_demand(
        connection_info->remote.node_id);
    }
    return send_data_status;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_supervision_handle_supervision_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Make sure the supervision timer is running if needed
  process_post(&zwave_command_class_supervision_process,
               ZWAVE_COMMAND_CLASS_SUPERVISION_START_TIMER,
               NULL);

  // We expect all fields up to "duration" to be present, also in v1.
  if (frame_length <= SUPERVISION_REPORT_DURATION_INDEX) {
    return SL_STATUS_FAIL;
  }
  // Verify that the Supervision Report corresponds to the session that
  // we are currently controlling.
  supervision_id_t supervision_id
    = zwave_command_class_supervision_find_session(
      frame_data[SUPERVISION_REPORT_SESSION_ID_INDEX]
        & SUPERVISION_REPORT_PROPERTIES1_SESSION_ID_MASK,
      connection_info->remote.node_id,
      connection_info->remote.endpoint_id);

  supervised_session_t *current_session
    = zwave_command_class_supervision_find_session_by_unique_id(supervision_id);

  // If we have no idea about this session, we will just ignore it.
  if (current_session == NULL) {
    // Maybe a duplicate transmission of a finished report.
    // Just ignore it happily.
    sl_log_debug(LOG_TAG,
                 "NodeID %d:%d sent us an unknown Supervision Session ID (%d)",
                 connection_info->remote.node_id,
                 connection_info->remote.endpoint_id,
                 frame_data[SUPERVISION_REPORT_SESSION_ID_INDEX]
                   & SUPERVISION_REPORT_PROPERTIES1_SESSION_ID_MASK);
    zwave_command_class_supervision_process_log();
    return SL_STATUS_OK;
  }

  // Look up what's in the frame, More Update flag, Status and Duration:
  bool more_updates
    = frame_data[SUPERVISION_REPORT_MORE_STATUS_INDEX]
      & SUPERVISION_REPORT_PROPERTIES1_MORE_STATUS_UPDATES_BIT_MASK;
  current_session->status = frame_data[SUPERVISION_REPORT_STATUS_INDEX];

  // Print out supervision session data
  sl_log_debug(LOG_TAG,
               "Incoming Supervision Report for NodeID %d:%d, group %d, "
               "Session ID (%d). Status: %d ",
               current_session->session.node_id,
               current_session->session.endpoint_id,
               current_session->session.group_id,
               current_session->session.session_id,
               current_session->status);

  if (frame_data[SUPERVISION_REPORT_DURATION_INDEX] < 0xFE) {
    current_session->expiry_time
      = clock_time()
        + zwave_duration_to_time(frame_data[SUPERVISION_REPORT_DURATION_INDEX])
        + SUPERVISION_REPORT_TIMEOUT;
  } else {  // Duration is unknown. Allocate some default waiting time here.
    current_session->expiry_time = clock_time()
                                   + SUPERVISION_DEFAULT_SESSION_DURATION
                                   + SUPERVISION_REPORT_TIMEOUT;
  }

  if (current_session->status == SUPERVISION_REPORT_WORKING) {
    if (more_updates == false) {
      sl_log_debug(LOG_TAG,
                   "NodeID %d:%d sent a WORKING status with the more_update "
                   "flag set to 0. Supervision Session ID (%d) will probably "
                   "time out automatically after its expiry duration",
                   connection_info->remote.node_id,
                   connection_info->remote.endpoint_id,
                   current_session->session.session_id);
    }
    // Wait for more if the node advertises more updates
    process_post(&zwave_command_class_supervision_process,
                 ZWAVE_COMMAND_CLASS_SUPERVISION_START_TIMER,
                 NULL);

    // Give the callback to the user, so that it knows that the node is working
    if (current_session->callback != NULL) {
      current_session->callback(
        current_session->status,
        current_session->tx_info_valid ? &current_session->tx_info : NULL,
        current_session->user);
    }

  } else {
    // Close this session if it is not WORKING
    if (current_session->callback != NULL) {
      current_session->callback(
        current_session->status,
        current_session->tx_info_valid ? &current_session->tx_info : NULL,
        current_session->user);
    }
    zwave_command_class_supervision_close_session(supervision_id);
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_supervision_control_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, it should have not come here.
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_SUPERVISION);

  switch (frame_data[COMMAND_INDEX]) {
    case SUPERVISION_REPORT:
      return zwave_command_class_supervision_handle_supervision_report(
        connection,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Callback functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Callback function registered to @ref zwave_tx used
 * to track when a Supervision frame has been sent successfully.
 *
 * @param status Refer to @ref on_zwave_tx_send_data_complete_t
 * @param tx_info Refer to @ref on_zwave_tx_send_data_complete_t
 * @param user Refer to @ref on_zwave_tx_send_data_complete_t
 */
void zwave_command_class_supervision_on_send_data_complete(
  uint8_t status, const zwapi_tx_report_t *tx_info, void *user)
{
  // At that point, we know TX has sent the frame
  // However, we rely on the Supervision Report to make the final callback.

  // We need the process to start a timer, waiting for a report
  // If the report was already received and processed, the timer will not do
  // anything bad as the session will be closed when expiring.
  process_post(&zwave_command_class_supervision_process,
               ZWAVE_COMMAND_CLASS_SUPERVISION_START_TIMER,
               NULL);

  supervision_id_t supervision_id = (intptr_t)user;

  supervised_session_t *ongoing_session
    = zwave_command_class_supervision_find_session_by_unique_id(supervision_id);

  if (ongoing_session == NULL) {
    // RX has already received the report and the session is closed.
    // Or somebody asked for a callback for Multicast, which won't happen just yet
    return;
  }

  // Verify that transmission was successful, else just callback a fail and close
  // the supervision session
  if ((status != TRANSMIT_COMPLETE_OK)
      && (status != TRANSMIT_COMPLETE_VERIFIED)) {
    if (ongoing_session->callback != NULL) {
      ongoing_session->callback(SUPERVISION_REPORT_FAIL,
                                tx_info,
                                ongoing_session->user);
    }
    zwave_command_class_supervision_close_session(supervision_id);
    return;
  }

  // Start waiting for a report
  // Verify here that we are not in the WORKING stage because sometimes the
  // report comes in before the send_data callback.
  if (ongoing_session->status != SUPERVISION_REPORT_WORKING) {
    ongoing_session->expiry_time = clock_time() + SUPERVISION_REPORT_TIMEOUT;
  }
  // Save the tx info for the user callback later on.
  ongoing_session->tx_info_valid = true;
  memcpy(&ongoing_session->tx_info, tx_info, sizeof(zwapi_tx_report_t));
}

///////////////////////////////////////////////////////////////////////////////
// Init function
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_supervision_init()
{
  // Start up the process for monitoring Supervision Sessions
  process_start(&zwave_command_class_supervision_process, NULL);

  // Clean up our "Wake on demand" list
  memset(wake_on_demand_list, 0, sizeof(zwave_nodemask_t));

  // Register Supervision CC handler to the Z-Wave CC framework
  zwave_command_handler_t handler = {};
  handler.support_handler    = &zwave_command_class_supervision_support_handler;
  handler.control_handler    = &zwave_command_class_supervision_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.command_class      = COMMAND_CLASS_SUPERVISION;
  handler.version            = SUPERVISION_VERSION_V2;
  handler.command_class_name = "Supervision";
  handler.manual_security_validation = false;

  return zwave_command_handler_register_handler(handler);
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_supervision_support_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, it should have not come here.
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_SUPERVISION);

  switch (frame_data[COMMAND_INDEX]) {
    case SUPERVISION_GET:
      return zwave_command_class_supervision_handle_supervision_get(
        connection,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t zwave_command_class_supervision_send_data(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data_payload,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_supervision_complete,
  void *user,
  zwave_tx_session_id_t *session)
{
  // Check if we can handle the frame size, else bail out.
  if (data_length > SUPERVISION_ENCAPSULATED_COMMAND_MAXIMUM_SIZE) {
    sl_log_warning(LOG_TAG,
                   "Frame too large (length = %d) to be Supervision "
                   "encapsulated. Discarding.",
                   data_length);
    return SL_STATUS_WOULD_OVERFLOW;
  }

  // Pick up the connection info and update the currently controlled session from it.
  supervision_id_t supervision_id
    = zwave_command_class_supervision_create_session(connection,
                                                     tx_options,
                                                     on_supervision_complete,
                                                     user);
  supervised_session_t *ongoing_session
    = zwave_command_class_supervision_find_session_by_unique_id(supervision_id);

  if (ongoing_session == NULL) {
    zwave_command_class_supervision_process_log();
    sl_log_warning(LOG_TAG,
                   "Cannot allocate Supervision ID for sending session. "
                   "Dropping send data call.");
    return SL_STATUS_FAIL;
  }

  // Make a new payload with the encapsulated frame.
  zwave_supervision_get_frame_t frame = {0};
  frame.command_class                 = COMMAND_CLASS_SUPERVISION;
  frame.command                       = SUPERVISION_GET;

  // We always ask for status updates.
  frame.status_wake_session_field
    = SUPERVISION_GET_PROPERTIES1_STATUS_UPDATES_BIT_MASK
      | ongoing_session->session.session_id;
  frame.encapsulated_command_length = data_length;
  memcpy(frame.encapsulated_command, data_payload, data_length);
  uint16_t supervision_frame_size
    = sizeof(frame) - SUPERVISION_ENCAPSULATED_COMMAND_MAXIMUM_SIZE
      + data_length;

  // Supervision Get expect 1 frame (or more) frames in response.
  // Update the tx option accordingly.
  zwave_tx_options_t supervision_tx_options  = *tx_options;
  supervision_tx_options.number_of_responses = tx_options->number_of_responses;
  supervision_tx_options.number_of_responses += 1;

  intptr_t user_parameter = (intptr_t)INVALID_SUPERVISION_ID;
  if (connection->remote.is_multicast == false) {
    user_parameter = (intptr_t)supervision_id;
  }
  sl_status_t zwave_tx_status = zwave_tx_send_data(
    connection,
    supervision_frame_size,
    (uint8_t *)&frame,
    &supervision_tx_options,
    connection->remote.is_multicast
      ? NULL
      : &zwave_command_class_supervision_on_send_data_complete,
    (void *)user_parameter,
    session);

  if (zwave_tx_status != SL_STATUS_OK) {
    // Abort the supervision session, no callback to the user
    zwave_command_class_supervision_close_session(supervision_id);
  } else if (session != NULL) {
    zwave_command_class_supervision_assign_session_tx_id(supervision_id,
                                                         *session);
  }
  //Ensure a timer is running for the newly created sessions.
  // (if we created mulitcast sessions, the session == NULL but we still
  // want to start that timer)
  process_post(&zwave_command_class_supervision_process,
               ZWAVE_COMMAND_CLASS_SUPERVISION_START_TIMER,
               NULL);

  return zwave_tx_status;
}

sl_status_t
  zwave_command_class_supervision_abort_send_data(zwave_tx_session_id_t session)
{
  // Just try to cancel the transmission at the TX level and
  // close the supervision session. Supervision status updates received subsequently
  // will be ignored
  sl_log_debug(LOG_TAG,
               "Attempting to abort Supervision "
               "sesssion associated with Tx Session %p",
               session);
  zwave_command_class_supervision_close_session_by_tx_session(session);
  return zwave_tx_abort_transmission(session);
}

sl_status_t
  zwave_command_class_supervision_wake_on_demand(zwave_node_id_t node_id)
{
  // Buffer overflow protection
  if ((node_id - 1) < 0 || ((node_id - 1) / 8) >= sizeof(zwave_nodemask_t)) {
    sl_log_debug(LOG_TAG,
                 "Wake Up On Demand requested on a node (%d) out of supported "
                 "range. Ignoring",
                 node_id);
    return SL_STATUS_WOULD_OVERFLOW;
  }

  // Add the NodeID to our Wake On Demand list.
  wake_on_demand_list[(node_id - 1) / 8] |= (1 << ((node_id - 1) % 8));
  return SL_STATUS_OK;
}

sl_status_t
  zwave_command_class_supervision_stop_wake_on_demand(zwave_node_id_t node_id)
{
  // Buffer overflow protection
  if ((node_id - 1) < 0 || ((node_id - 1) / 8) >= sizeof(zwave_nodemask_t)) {
    return SL_STATUS_WOULD_OVERFLOW;
  }

  // Just remove the node from the Wake On Demand list:
  wake_on_demand_list[(node_id - 1) / 8] &= (0xFF - (1 << ((node_id - 1) % 8)));
  return SL_STATUS_OK;
}
