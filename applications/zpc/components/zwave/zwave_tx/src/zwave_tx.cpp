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
#include "zwave_tx.h"
#include "zwave_tx_queue.h"
#include "zwave_tx_callbacks.h"
#include "zwave_tx_process.h"

// Includes from other components
#include "sl_log.h"
#include "zwave_tx_groups.h"
#include "zwave_controller_callbacks.h"
#include "zwapi_protocol_mem.h"

// Generic includes
#include <cstring>  // Using memcpy

// Setup the logging
constexpr char LOG_TAG[] = "zwave_tx";

// zwave_tx_process.cpp owns the tx_queue
extern zwave_tx_queue tx_queue;

static const zwave_controller_callbacks_t zwave_tx_callbacks = {
  .on_new_network_entered = &zwave_tx_on_new_network_entered,
  .on_rx_frame_received   = &zwave_tx_on_frame_received,
};

sl_status_t
  zwave_tx_send_data(const zwave_controller_connection_info_t *connection,
                     uint16_t data_length,
                     const uint8_t *data,
                     const zwave_tx_options_t *tx_options,
                     const on_zwave_tx_send_data_complete_t on_send_complete,
                     void *user,
                     zwave_tx_session_id_t *session)
{
  // If we are in the middle of a Tx Queue flush, do not accept new elements
  if (zwave_tx_process_queue_flush_is_ongoing() == true) {
    sl_log_info(LOG_TAG, "Rejecting new frame due to Tx Queue flush.");
    return SL_STATUS_FAIL;
  }

  // Copy all the user parameters in a queue element
  zwave_tx_queue_element_t new_element = {0};

  // Frame length validation.
  if (data_length > sizeof(new_element.data)) {
    sl_log_error(LOG_TAG,
                 "Frame too large to be sent (size=%d). Aborting.",
                 data_length);
    return SL_STATUS_FAIL;
  }
  // The length has to be >0 for regular frames (test frames do not need a payload)
  if (data_length == 0 && tx_options->is_test_frame == false) {
    sl_log_error(LOG_TAG,
                 "Empty frame sent to the Tx Queue. (size=%d). Ignoring.",
                 data_length);
    return SL_STATUS_FAIL;
  }

  new_element.connection_info = *connection;
  new_element.data_length     = data_length;
  if (new_element.data_length > 0) {
    memcpy(&new_element.data, data, data_length);
  }
  new_element.zwave_tx_options  = *tx_options;
  new_element.data_length       = data_length;
  new_element.callback_function = on_send_complete;
  new_element.user              = user;

  // Is there a parent element?
  if (new_element.zwave_tx_options.valid_parent_session_id == true) {
    // Fetch the parent data
    zwave_tx_queue_element_t parent_element = {0};
    sl_status_t fetch_parent_status;
    fetch_parent_status
      = tx_queue.get_by_id(&parent_element,
                           new_element.zwave_tx_options.parent_session_id);
    if (fetch_parent_status != SL_STATUS_OK) {
      sl_log_warning(LOG_TAG,
                     "Non-existing parent session ID (id=%p) for incoming TX "
                     "Queue frame. Discarding incoming frame.\n",
                     new_element.zwave_tx_options.parent_session_id);
      return SL_STATUS_FAIL;
    }

    // Child must be sent before the parent.
    if ((new_element.zwave_tx_options.qos_priority
         & (ZWAVE_TX_QOS_MAX_PRIORITY))
        == ZWAVE_TX_QOS_MAX_PRIORITY) {
      parent_element.zwave_tx_options.qos_priority
        = new_element.zwave_tx_options.qos_priority;
    } else {
      new_element.zwave_tx_options.qos_priority
        = parent_element.zwave_tx_options.qos_priority + 1;
    }
  }

  // Local cannot be multicast when transmitting:
  new_element.connection_info.local.is_multicast = false;

  // If the caller passed local NodeID to 0, we replace it with the real value
  if (new_element.connection_info.local.node_id == 0) {
    new_element.connection_info.local.node_id
      = zwave_network_management_get_node_id();
  }

  // Do not try to transmit to our own NodeID, it goes wrong
  // and takes long to recover from.
  if ((new_element.connection_info.remote.is_multicast == false)
      && (new_element.connection_info.remote.node_id
          == zwave_network_management_get_node_id())) {
    sl_log_warning(LOG_TAG,
                   "Rejecting frame with our own NodeID as destination (%d)",
                   new_element.connection_info.remote.node_id);
    return SL_STATUS_FAIL;
  }

  // FIXME: There is probably a better place to do this.
  // Protocol CCs must not be encrypted, but Z-Wave API transport is called last
  // and S2 will just encrypt stuff..
  // We automatically downgrade security if Command Class is 0x00 or 0x01.
  if (data_length > 0 && (data[0] == 0x00 || data[0] == 0x01)) {
    new_element.connection_info.encapsulation
      = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
    new_element.connection_info.remote.endpoint_id = 0;
  }

  if (SL_STATUS_OK == tx_queue.enqueue(new_element, session)) {
    process_post(&zwave_tx_process, ZWAVE_TX_SEND_NEXT_MESSAGE, 0);
    return SL_STATUS_OK;
  }

  sl_log_error(LOG_TAG, "Tx Queue rejected new frame request.");
  return SL_STATUS_FAIL;
}

sl_status_t zwave_tx_send_test_frame(
  zwave_node_id_t destination_node_id,
  rf_power_level_t power_level,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session)
{
  // Prepare the tx_options / connection_info automatically
  zwave_tx_options_t tx_options = {.number_of_responses = 0,
                                   .discard_timeout_ms  = 0,
                                   .qos_priority = ZWAVE_TX_QOS_MIN_PRIORITY,
                                   .parent_session_id       = 0,
                                   .valid_parent_session_id = 0,
                                   .is_test_frame           = true,
                                   .rf_power                = power_level,
                                   .group_id           = ZWAVE_TX_INVALID_GROUP,
                                   .is_first_follow_up = false,
                                   .send_follow_ups    = false};

  zwave_controller_connection_info_t connection = {};
  connection.remote.node_id                     = destination_node_id;
  connection.remote.endpoint_id                 = 0;
  connection.remote.is_multicast                = false;
  connection.local.node_id                      = 0;
  connection.local.endpoint_id                  = 0;
  connection.local.is_multicast                 = false;
  connection.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;

  // Enqueue like a normal frame
  return zwave_tx_send_data(&connection,
                            0,
                            NULL,
                            &tx_options,
                            on_send_complete,
                            user,
                            session);
}

sl_status_t zwave_tx_abort_transmission(zwave_tx_session_id_t session_id)
{
  return zwave_tx_process_abort_transmission(session_id);
}

uint8_t zwave_tx_get_number_of_responses(zwave_tx_session_id_t session)
{
  return tx_queue.get_number_of_responses(session);
}

sl_status_t zwave_tx_init()
{
  // register callbacks to the Z-Wave Controller
  zwave_controller_register_callbacks(&zwave_tx_callbacks);

  // Tell the Z-Wave Controller that we want to do something on Reset
  zwave_controller_register_reset_step(
    &zwave_tx_process_flush_queue_reset_step,
    ZWAVE_CONTROLLER_TX_FLUSH_RESET_STEP_PRIORITY);
  return SL_STATUS_OK;
}

void zwave_tx_log_queue(bool with_contents)
{
  zwave_tx_process_log_state();
  tx_queue.log(with_contents);
}
