
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
#include "zwave_tx.h"
#include "zwave_tx_process.h"
#include "zwave_tx_callbacks.h"
#include "zwave_tx_queue.h"
#include "zwave_tx_state_logging.h"
#include "zwave_tx_intercept_payload.h"
#include "zwave_tx_follow_ups.h"

// Generic includes
#include <vector>

// Includes from other components
#include "sl_log.h"
#include "zwave_tx_groups.h"
#include "zwave_controller_internal.h"

// Contiki includes
#include "lib/assert.h"

#define LOG_TAG "zwave_tx_process"

/// The maximum time in seconds that we will wait for
/// a \ref zwapi_send_data callback from the \ref ZWAPI
#define SEND_DATA_EMERGENCY_TIMEOUT 65
/// The time to backoff (in milliseconds) after aborting a transmission with the Z-Wave API.
#define BACKOFF_AFTER_TRANSMISSION_ABORT 200

// Our local variables.
static zwave_tx_state_t state;
static struct etimer emergency_timer;
static struct etimer backoff_timer;
static bool reset_awaiting_empty_queue;

// The Z-Wave TX queue
zwave_tx_queue tx_queue;  // zwave_tx.cpp uses the tx_queue.

// The frame currently being handled
zwave_tx_session_id_t current_tx_session_id;

/**
 * @brief Used if the current message should be dropped
 */
static void zwave_tx_drop_unsent_current_message()
{
  sl_log_warning(LOG_TAG, "Dropping frame (id=%p)\n", current_tx_session_id);
  // Ensure that it won't be requeued due to Fast Track option.
  tx_queue.disable_fasttack(current_tx_session_id);
  // Generate a callback for it, so that it will be cleaned up from the queue
  zwave_tx_send_data_callback(TRANSMIT_COMPLETE_FAIL, nullptr);
}

/**
 * @brief Verifies if a frame is fully processed or needs
 * to be discarded.
 *
 * The following checks are performed:
 * 1. If frame transmission not initiated, has the discard timer expired ?
 * 2. Has frame transmission been initiated and we received a callback
 *    but it's still on top of the queue?
 * 3. Has frame transmission been initiated and we are waiting for a callback ?
 *
 * @returns true  if the frame is still to be sent.
 * @returns false if the frame has been processed and
 *                can be removed from the queue.
 */
static bool is_frame_to_be_sent(const zwave_tx_queue_element_t &e)
{
  // Verify if the frame has expired and was not sent yet.
  if ((e.zwave_tx_options.discard_timeout_ms > 0)
      && (e.transmission_timestamp == 0)
      && ((e.queue_timestamp + e.zwave_tx_options.discard_timeout_ms)
          < clock_time())) {
    sl_log_info(LOG_TAG, "Frame discard timeout has expired.\n");
    zwave_tx_drop_unsent_current_message();
    return false;
  }

  // Has the frame been fully sent ?
  // Happens after a send_data callback has arrived for a frame or its parent.
  if (e.transmission_time != 0) {
    // Frame has been sent via children frames. Remove without sending.
    tx_queue.disable_fasttack(e.zwave_tx_session_id);
    process_post(&zwave_tx_process, ZWAVE_TX_SEND_OPERATION_COMPLETE, 0);
    return false;
  }

  // Has the frame been pushed to the Z-Wave transport or
  // Z-Wave API and we are waiting for a callback?
  if (e.transmission_timestamp != 0) {
    // Just wait, back to idle, and trust zwave_controller_transport_send_data !!
    sl_log_warning(LOG_TAG,
                   "Frame (id=%p) waiting for transport callback... "
                   "Tx Queue will not do anything until then.",
                   current_tx_session_id);
    state = ZWAVE_TX_STATE_IDLE;
    return false;
  }

  // Frame seems to be ready for transmission.
  return true;
}

/**
 * @brief Inspect the highest priority element in the queue and initiate sending.
 */
static void zwave_tx_process_send_next_message_step()
{
  if (tx_queue.empty()) {
    return;
  }

  const zwave_tx_queue_element_t e = *tx_queue.begin();
  state                            = ZWAVE_TX_STATE_TRANSMISSION_ONGOING;
  current_tx_session_id            = e.zwave_tx_session_id;

  if (is_frame_to_be_sent(e) == false) {
    return;
  }

  // Inspecting the payload data.
  sl_status_t zwave_tx_intercept_status = zwave_tx_intercept_payload(e);
  if (zwave_tx_intercept_status == SL_STATUS_OK) {
    tx_queue.set_transmission_timestamp(current_tx_session_id);
    etimer_set(&emergency_timer, SEND_DATA_EMERGENCY_TIMEOUT * CLOCK_SECOND);
    return;
  } else if (zwave_tx_intercept_status != SL_STATUS_NOT_SUPPORTED) {
    sl_log_warning(LOG_TAG,
                   "Intercept transmission failed, dropping current message");
    zwave_tx_drop_unsent_current_message();
    return;
  }

  sl_status_t transport_status = zwave_controller_transport_send_data(
    &e.connection_info,
    e.data_length,
    e.data,
    &e.zwave_tx_options,
    on_zwave_transport_send_data_complete,
    current_tx_session_id,
    current_tx_session_id);

  if (transport_status == SL_STATUS_OK) {
    tx_queue.set_transmission_timestamp(current_tx_session_id);
    state = ZWAVE_TX_STATE_IDLE;
    zwave_tx_process_check_queue();
    return;
  } else if (transport_status == SL_STATUS_WOULD_OVERFLOW) {
    // Frame cannot be handled.
    zwave_tx_drop_unsent_current_message();
    return;
  } else if (transport_status != SL_STATUS_NOT_SUPPORTED) {
    // Here we are going to get into a stall...
    // Likely a higher priority frame queued shortcutting the encapsulation frames.
    // Discard the "Excess" frames for now.
    sl_log_warning(
      LOG_TAG,
      "Transport cannot handle new frame. Discarding to avoid a stall.");
    zwave_tx_drop_unsent_current_message();
    return;
  }

  // Initiate the transmission, sending the frame to the Z-Wave API
  uint8_t zwapi_tx_options = 0;

  // Singlecast or broadcast frame
  if (e.connection_info.remote.is_multicast == false) {
    if (e.connection_info.remote.node_id != ZWAVE_BROADCAST_NODE_ID) {
      // Singlecast are sent with Ack Request.
      zwapi_tx_options = TRANSMIT_OPTION_ACK;
      if (e.zwave_tx_options.fasttrack == false) {
        // Route resolution only if not in "fasttrack" mode
        zwapi_tx_options
          |= TRANSMIT_OPTION_AUTO_ROUTE | TRANSMIT_OPTION_EXPLORE;
      }
    }

    sl_status_t transmit_status;
    if (e.zwave_tx_options.is_test_frame == true) {
      transmit_status
        = zwapi_send_test_frame(e.connection_info.remote.node_id,
                                e.zwave_tx_options.rf_power,
                                zwave_tx_send_test_frame_callback);
    } else {
      transmit_status = zwapi_send_data(e.connection_info.remote.node_id,
                                        e.data,
                                        e.data_length,
                                        zwapi_tx_options,
                                        zwave_tx_send_data_callback);
    }

    if (SL_STATUS_OK == transmit_status) {
      // Record transmission timestamp and start emergency timer
      tx_queue.set_transmission_timestamp(current_tx_session_id);
      etimer_set(&emergency_timer, SEND_DATA_EMERGENCY_TIMEOUT * CLOCK_SECOND);
    } else {
      sl_log_warning(LOG_TAG,
                     "Z-Wave API zwapi_send_data/zwapi_send_test_frame refused "
                     "frame (id = %p)\n",
                     current_tx_session_id);
      zwave_tx_drop_unsent_current_message();
    }  // End of singlecast frame
    return;
  }

  // Multicast frame
  if (e.connection_info.remote.is_multicast == true) {
    // Verify that transports have done their job
    if (e.connection_info.encapsulation
        != ZWAVE_CONTROLLER_ENCAPSULATION_NONE) {
      sl_log_error(LOG_TAG,
                   "Encrypted frame was requested to be sent as multicast. "
                   "Dropping frame.\n");
      zwave_tx_drop_unsent_current_message();
      return;
    }

    // No Ack, route or explore frames for tx_options.
    // We don't care about fasttrack here.
    // If the tx_options indicate a group ID for a multicast frame
    // (other than the connection_info.remote), it means that it is a multicast
    // frame to be sent as a broadcast
    if (e.zwave_tx_options.group_id != ZWAVE_TX_INVALID_GROUP) {
      zwapi_tx_options |= TRANSMIT_OPTION_MULTICAST_AS_BROADCAST;
    }

    zwave_nodemask_t node_list = {};
    if (SL_STATUS_OK
        != zwave_tx_get_nodes(node_list,
                              e.connection_info.remote.multicast_group)) {
      sl_log_debug(LOG_TAG,
                   "Z-Wave Tx cannot read Node List for Group ID %d. Dropping "
                   "frame (id = %p)\n",
                   e.connection_info.remote.multicast_group,
                   current_tx_session_id);
      zwave_tx_drop_unsent_current_message();
      return;
    }
    if (SL_STATUS_OK
        == zwapi_send_data_multi(node_list,
                                 e.data,
                                 e.data_length,
                                 zwapi_tx_options,
                                 zwave_tx_send_data_multi_callback)) {
      etimer_set(&emergency_timer, SEND_DATA_EMERGENCY_TIMEOUT * CLOCK_SECOND);
    } else {
      sl_log_warning(
        LOG_TAG,
        "Z-Wave API zwapi_send_data_multi refused frame (id = %p)\n",
        current_tx_session_id);
      zwave_tx_drop_unsent_current_message();
    }
    return;
  }  // End of multicast frame
}

/**
 * @brief Looks at transmissions results and decides what to do next
 *
 * TODO: this function should also provide duty cycle monitoring
 * TODO: this is the correct place to collect IMA data.
 *
 * If the transmission was successful, 2 choices:
 * 1. If we expect some replies: Apply a backoff before the next transmission
 * 2. If we do not expect a reply: Remove the element from the queue and move on to the next one.
 *
 * If the transmission was not successful:
 * 1. If it was a fastrack qos: requeue without fasttrack
 * 2. Else just give up for this element, remove it from the queue.
 */
static void zwave_tx_message_transmission_completed_step()
{
  state = ZWAVE_TX_STATE_IDLE;
  etimer_stop(&emergency_timer);

  if (reset_awaiting_empty_queue == true) {
    tx_queue.clear();
    reset_awaiting_empty_queue = false;
    sl_log_info(LOG_TAG, "Reset step: Tx Queue flush completed");
    zwave_controller_on_reset_step_complete(
      ZWAVE_CONTROLLER_TX_FLUSH_RESET_STEP_PRIORITY);
  }

  if (tx_queue.empty()) {
    sl_log_warning(
      LOG_TAG,
      "Callback for a non-existing queue element. (queue is empty). "
      "Frame was transmitted but the caller will "
      "get no callback. Session ID: %p\n",
      current_tx_session_id);
    return;
  }

  zwave_tx_queue_element_t completed_element = {0};
  if (SL_STATUS_OK
      != tx_queue.get_by_id(&completed_element, current_tx_session_id)) {
    sl_log_warning(LOG_TAG,
                   "Callback for a non-existing queue element. "
                   "Cannot find the following Session ID: %p\n",
                   current_tx_session_id);
    zwave_tx_process_check_queue();
    return;
  }

  // Did the element fail and need to be requeued ? (fasttrack)
  // We
  if (completed_element.send_data_status != TRANSMIT_COMPLETE_OK
      && (completed_element.zwave_tx_options.fasttrack == true)) {
    sl_log_info(LOG_TAG,
                "Fastrack transmit attempt failed. Requeueing element %p\n",
                completed_element.zwave_tx_session_id);
    tx_queue.disable_fasttack(completed_element.zwave_tx_session_id);
    // Reset the transmission timestamp, so that it gets discarded
    // if has spent too long in the queue from enqueuing to 2nd transmit attempt.
    tx_queue.reset_transmission_timestamp(
      completed_element.zwave_tx_session_id);
    zwave_tx_process_check_queue();
    return;
  }

  // Do we need to send some follow-up frames ?
  if ((completed_element.connection_info.remote.is_multicast == true)
      && (completed_element.zwave_tx_options.send_follow_ups == true)
      && (SL_STATUS_OK
          == zwave_tx_enqueue_follow_ups(
            completed_element.zwave_tx_session_id))) {
    zwave_tx_process_check_queue();
    return;
  }

  // Call the callback function, if there is one registered for the element
  if (completed_element.callback_function) {
    completed_element.callback_function(
      completed_element.send_data_status,
      &(completed_element.send_data_tx_status),
      completed_element.user);
  }

  // Apply backoff if it was successful singlecast and we expect replies.
  if (completed_element.send_data_status == TRANSMIT_COMPLETE_OK
      && completed_element.zwave_tx_options.number_of_responses > 0
      && completed_element.connection_info.remote.is_multicast == false
      && completed_element.connection_info.remote.node_id
           != ZWAVE_BROADCAST_NODE_ID) {
    clock_time_t backoff_time
      = completed_element.zwave_tx_options.number_of_responses
        * (completed_element.transmission_time + CLOCK_SECOND);
    sl_log_debug(LOG_TAG,
                 "Starting Z-Wave TX backoff for frame sent to NodeID %d, "
                 "backoff time: %lu ms.\n",
                 completed_element.connection_info.remote.node_id,
                 backoff_time);
    state = ZWAVE_TX_STATE_BACKOFF;
    // The backoff time should be transmission_time + 1 second.
    // Details are available in the Role Type specification, section "Node interview and response timeouts"
    etimer_set(&backoff_timer, backoff_time);
    // Here we will just return without removing the element from the queue
    // just yet, so that we can match it with a response if we receive one.
    return;
  }

  // We are done (for ever!) with the element, so we delete it from the queue.
  tx_queue.pop(current_tx_session_id);
  // Send the next message if we are idle and the queue is not empty
  zwave_tx_process_check_queue();
}

static void zwave_tx_message_emergency_timeout_step()
{
  sl_log_error(LOG_TAG,
               "Emergency timer fired! Aborting transmission %p\n",
               current_tx_session_id);
  zwave_tx_send_data_callback(TRANSMIT_COMPLETE_FAIL, nullptr);
}

static void zwave_tx_resume_from_backoff_step()
{
  // Ensure that the backoff timer is not running anymore
  etimer_stop(&backoff_timer);
  // Finally remove the element that caused the backoff
  tx_queue.pop(current_tx_session_id);
  // Go back to idle and verify if there is more in the queue.
  state = ZWAVE_TX_STATE_IDLE;
  zwave_tx_process_check_queue();
}

sl_status_t
  zwave_tx_process_abort_transmission(zwave_tx_session_id_t session_id)
{
  // Find out all children for this frame
  std::vector<zwave_tx_session_id_t> frames_to_abort = {};
  zwave_tx_queue_element_t child_element             = {};
  uint32_t child_index                               = 0;
  while (SL_STATUS_OK
         == tx_queue.get_child_by_id(&child_element, session_id, child_index)) {
    frames_to_abort.push_back(child_element.zwave_tx_session_id);
    child_index += 1;
  }

  // Then abort all the children
  sl_status_t removal_status = SL_STATUS_OK;
  for (auto it = frames_to_abort.cbegin(); it != frames_to_abort.cend(); ++it) {
    removal_status |= zwave_tx_process_abort_transmission(*it);
  }

  // Remove frames by stopping send_data and pop it from the queue.
  sl_log_info(LOG_TAG, "Aborting transmission of element %p\n", session_id);
  // This happens only in case of manual call to zwave_tx_abort_transmission()
  // Because discard timeouts are ignored after initiating a transmission.
  if (state == ZWAVE_TX_STATE_TRANSMISSION_ONGOING
      && current_tx_session_id == session_id) {
    zwapi_abort_send_data();
    // After aborting, the Z-Wave API will still invoke the callback,
    // so we keep this session_id in the queue and wait for it to terminate.
    return SL_STATUS_IN_PROGRESS;
  }

  // Else if no child and not ongoing, just remove the message from the queue.
  removal_status |= tx_queue.pop(session_id);
  return removal_status;
}

static void zwave_tx_inspect_received_frame(
  zwave_controller_connection_info_t *connection_info)
{
  zwave_tx_queue_element_t current_element;
  if (SL_STATUS_OK
      != tx_queue.get_by_id(&current_element, current_tx_session_id)) {
    return;
  }

  // We only match the destination nodeID/endpoint, not the payload.
  if (connection_info->remote.node_id
        == current_element.connection_info.remote.node_id
      && connection_info->remote.endpoint_id
           == current_element.connection_info.remote.endpoint_id) {
    // Is that the last or do we need more replies?
    if (current_element.zwave_tx_options.number_of_responses <= 1) {
      //We got the last needed reply:
      sl_log_debug(LOG_TAG,
                   "Received all expected replies from NodeID "
                   "%d for frame id=%p. Resuming from TX back off\n",
                   connection_info->remote.node_id,
                   current_element.zwave_tx_session_id);
      zwave_tx_resume_from_backoff_step();
    }

    // Decrement the number of expected responses and wait for the next one.
    tx_queue.decrement_expected_responses(current_tx_session_id);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Functions shared among the component
////////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_tx_flush_queue_reset_step()
{
  sl_log_info(LOG_TAG, "Reset step: Initiating flush of the Tx Queue");

  // If we are sending. Abort the current session first and wait for the transport
  // to callback.
  if (ZWAVE_TX_STATE_TRANSMISSION_ONGOING == state) {
    sl_status_t abort_status
      = zwave_tx_process_abort_transmission(current_tx_session_id);
    if (abort_status == SL_STATUS_IN_PROGRESS) {
      reset_awaiting_empty_queue = true;
      // Return OK to tell the controller we contact them back when ready.
      return SL_STATUS_OK;
    }
  }
  // If we could abort without delay or were not sending,
  // just tell the controller to move on. Return something different than
  // SL_STATUS_OK, reset can resume without waiting for a callback
  tx_queue.clear();
  sl_log_info(LOG_TAG, "Reset step: Tx Queue flush completed");
  return SL_STATUS_EMPTY;
}

void zwave_tx_process_check_queue()
{
  if (ZWAVE_TX_STATE_IDLE == state && !tx_queue.empty()) {
    process_post(&zwave_tx_process, ZWAVE_TX_SEND_NEXT_MESSAGE, 0);
  }
}

void zwave_tx_process_log_state()
{
  sl_log_debug(
    LOG_TAG,
    "Current Z-Wave Tx State: %s - Current(/last) Tx Session (id=%p) \n",
    zwave_tx_state_name(state),
    current_tx_session_id);
}

////////////////////////////////////////////////////////////////////////////////
// Contiki Process
////////////////////////////////////////////////////////////////////////////////
PROCESS(zwave_tx_process, "zwave_tx_process");

PROCESS_THREAD(zwave_tx_process, ev, data)
{
  PROCESS_BEGIN();

  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      state = ZWAVE_TX_STATE_IDLE;
      zwave_tx_init();
      reset_awaiting_empty_queue = false;
      sl_log_info(LOG_TAG, "Z-Wave TX process initialized.\n");

    } else if (ev == PROCESS_EVENT_EXIT) {
      state = ZWAVE_TX_STATE_IDLE;
      zwave_tx_shutdown();
      sl_log_info(LOG_TAG, "Z-Wave TX process exited.\n");

    } else if (ev == PROCESS_EVENT_EXITED) {
      // Do not do anything with this event, just wait to go down.

    } else if (ev == ZWAVE_TX_SEND_NEXT_MESSAGE) {
      // If we are not idle, silently ignore the next message event.
      // We look at the queue again when going back to idle.
      if (state == ZWAVE_TX_STATE_IDLE) {
        zwave_tx_process_send_next_message_step();
      }

    } else if (ev == ZWAVE_TX_SEND_OPERATION_COMPLETE
               && state == ZWAVE_TX_STATE_TRANSMISSION_ONGOING) {
      zwave_tx_message_transmission_completed_step();

    } else if (ev == PROCESS_EVENT_TIMER) {
      if (state == ZWAVE_TX_STATE_TRANSMISSION_ONGOING
          && data == &emergency_timer) {
        zwave_tx_message_emergency_timeout_step();

      } else if (state == ZWAVE_TX_STATE_BACKOFF && data == &backoff_timer) {
        sl_log_debug(LOG_TAG,
                     "Tx backoff timer expired. Did not get all expected "
                     "answer(s) for frame id=%p.\n",
                     current_tx_session_id);
        zwave_tx_resume_from_backoff_step();
      }
    } else if (ev == ZWAVE_TX_FRAME_RECEIVED) {
      // If we are not in backoff, ignore incoming Z-Wave frames.
      if (state == ZWAVE_TX_STATE_BACKOFF) {
        zwave_tx_inspect_received_frame(
          (zwave_controller_connection_info_t *)data);
      }
    } else {
      // If an event/state combination does not make sense, the
      // state machine may get stuck, so we print out a warning when
      // such an exception occurs.
      sl_log_warning(LOG_TAG,
                     "Unhandled Z-Wave Tx Event: %s while in State: %s\n",
                     zwave_tx_event_name(ev),
                     zwave_tx_state_name(state));
    }

    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
