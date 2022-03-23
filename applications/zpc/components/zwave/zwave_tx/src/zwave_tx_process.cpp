
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
#include "zwave_tx_process.h"
#include "zwave_tx_callbacks.h"
#include "zwave_tx_queue.h"
#include "zwave_tx_state_logging.h"
#include "zwave_tx_follow_ups.h"

// Generic includes
#include <vector>

// Includes from other components
#include "sl_log.h"
#include "zwave_tx_groups.h"
#include "zwave_controller_internal.h"
#include "zwave_controller_transport.h"

// Helper macro to determine if a transmit status indicates that the tramission
// was successful.
#define IS_TRANSMISSION_SUCCESSFUL(status) \
  ((status == TRANSMIT_COMPLETE_OK) || (status == TRANSMIT_COMPLETE_VERIFIED))

// Constants
static constexpr char LOG_TAG[] = "zwave_tx_process";

// Our private variables.
namespace
{
// Tx Queue state
zwave_tx_state_t state;
// back-off timer, used to decide when to send the next frame
struct etimer backoff_timer;
// Private variable indicating if a Tx Queue flush is ongoing
bool queue_flush_ongoing = false;
// The frame currently being handled
zwave_tx_session_id_t current_tx_session_id;
}  // namespace

// The Z-Wave TX queue
zwave_tx_queue tx_queue;  // zwave_tx.cpp uses the tx_queue.

// Forward declarations
static void zwave_tx_message_transmission_completed_step(
  zwave_tx_session_id_t session_id);

/**
 * @brief Used if the current message should be dropped
 */
static void zwave_tx_drop_unsent_current_message()
{
  sl_log_warning(LOG_TAG, "Dropping frame (id=%p)\n", current_tx_session_id);
  // Ensure that it won't be requeued due to Fast Track option.
  tx_queue.disable_fasttack(current_tx_session_id);
  // Generate a callback for it, so that it will be cleaned up from the queue
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_FAIL,
                                        nullptr,
                                        current_tx_session_id);
}

/**
 * @brief Checks if we return to IDLE or stay in TRANSMISSION_ONGOING
 * when an element is completed. Removes the element from the queue.
 *
 * @param session_id The session ID to remove from the queue.
 */
static void zwave_tx_finalize_element(zwave_tx_session_id_t session_id)
{
  zwave_tx_queue_element_t completed_element = {};
  if (SL_STATUS_OK != tx_queue.get_by_id(&completed_element, session_id)) {
    sl_log_warning(LOG_TAG,
                   "Cannot find the following Session ID: %p, "
                   "it is already removed from the Tx Queue. "
                   "Nothing more will happen.\n",
                   session_id);
    state = ZWAVE_TX_STATE_IDLE;
    zwave_tx_process_check_queue();
    return;
  }

  // Stay in Transmission ongoing or go back to idle?
  if ((completed_element.zwave_tx_options.valid_parent_session_id == true)
      && (SL_STATUS_OK
          == tx_queue.get_by_id(
            &completed_element,
            completed_element.zwave_tx_options.parent_session_id))) {
    // Keep the transmission "ONGOING" if the frame has parents
    // that are still in the queue, set the parent as ongoing.
    state                 = ZWAVE_TX_STATE_TRANSMISSION_ONGOING;
    current_tx_session_id = completed_element.zwave_tx_session_id;
  } else {
    // Go back to idle if we are done with a "chain" of frames.
    state = ZWAVE_TX_STATE_IDLE;
  }
  // We are done (for ever!) with the element, so we delete it from the queue.
  sl_log_debug(LOG_TAG, "Removing id=%p from the queue", session_id);
  tx_queue.pop(session_id);

  // Did we just reach an empty queue while waiting for a queue flush?
  if (zwave_tx_process_queue_flush_is_ongoing() && tx_queue.empty()) {
    sl_log_info(LOG_TAG, "Reset step: Tx Queue flush completed");
    zwave_controller_on_reset_step_complete(
      ZWAVE_CONTROLLER_TX_FLUSH_RESET_STEP_PRIORITY);
  }
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
    sl_log_warning(LOG_TAG, "Frame discard timeout has expired.\n");
    zwave_tx_drop_unsent_current_message();
    return false;
  }

  // Has the frame been fully sent ?
  // Happens after a send_data callback
  if (e.transmission_time != 0) {
    zwave_tx_message_transmission_completed_step(e.zwave_tx_session_id);
    return false;
  }

  // Has the frame been pushed to the Z-Wave transport or
  // Z-Wave API and we are waiting for a callback?
  if (e.transmission_timestamp != 0) {
    // Just wait, back to idle, and trust the transport to make its callback !!
    sl_log_debug(LOG_TAG,
                 "Frame (id=%p) waits for new elements or transport callback..."
                 "Tx Queue will not do anything until then.",
                 current_tx_session_id);
    return false;
  }

  // Frame seems to be ready for transmission.
  return true;
}

static sl_status_t zwave_tx_process_fetch_next_element_for_ongoing_transmission(
  zwave_tx_queue_element_t *next_element)
{
  // Take the highest priority child, from the current element.
  sl_status_t fetch_status
    = tx_queue.get_highest_priority_child(next_element, current_tx_session_id);
  if (fetch_status == SL_STATUS_OK) {
    return SL_STATUS_OK;
  }

  // No child, check if the current element is in the queue and needs processing
  fetch_status = tx_queue.get_by_id(next_element, current_tx_session_id);
  if (fetch_status == SL_STATUS_OK) {
    return SL_STATUS_OK;
  }

  // Current element does not exist?? Go back to IDLE.
  sl_log_warning(LOG_TAG,
                 "current_tx_session_id is not in the Tx Queue while state is "
                 "%s. Returning to IDLE.",
                 current_tx_session_id,
                 zwave_tx_state_name(state));
  state = ZWAVE_TX_STATE_IDLE;
  zwave_tx_process_check_queue();
  return SL_STATUS_NOT_FOUND;
}

/**
 * @brief Inspect the highest priority element in the queue and initiate sending.
 */
static void zwave_tx_process_send_next_message_step()
{
  if (tx_queue.empty()) {
    return;
  }

  zwave_tx_queue_element_t current_element = {};

  // Are we already sending? then try to see if we have children frame to send
  if (state == ZWAVE_TX_STATE_TRANSMISSION_ONGOING) {
    // Take the child with highest priority, else the current element if no child.
    if (SL_STATUS_OK
        != zwave_tx_process_fetch_next_element_for_ongoing_transmission(
          &current_element)) {
      return;
    }
  } else if (state == ZWAVE_TX_STATE_IDLE) {
    // We are idle, take the highest priority element from the queue.
    current_element = *tx_queue.begin();
    state           = ZWAVE_TX_STATE_TRANSMISSION_ONGOING;
  } else {
    // Other tx queue states should not try to call this function!
    sl_log_warning(LOG_TAG,
                   "Trying to send next element while Tx Queue state = %s. "
                   "This should not happen. Ignoring.",
                   zwave_tx_state_name(state));
    return;
  }
  // Save the current Session ID.
  current_tx_session_id = current_element.zwave_tx_session_id;

  if (is_frame_to_be_sent(current_element) == false) {
    return;
  }

  // Ask the transports to take the frame.
  sl_status_t transport_status = zwave_controller_transport_send_data(
    &(current_element.connection_info),
    current_element.data_length,
    current_element.data,
    &(current_element.zwave_tx_options),
    &on_zwave_transport_send_data_complete,
    current_tx_session_id,
    current_tx_session_id);

  if (transport_status == SL_STATUS_OK) {
    tx_queue.set_transmission_timestamp(current_tx_session_id);
    return;
  } else {
    // Here we are going to get into a stall...
    // Can happen e.g. if the transport tries to queue more but the Tx Queue
    // just refused the frame.
    // Or the transport cannot handle the new frame
    // Discard the "Excess" frames for now.
    sl_log_warning(LOG_TAG,
                   "Transport cannot handle new frame (id=%p). "
                   "Discarding to avoid a stall.",
                   current_tx_session_id);
    zwave_tx_drop_unsent_current_message();
    return;
  }
}

/**
 * @brief Looks at transmissions results and decides what to do next
 *
 * If the transmission was successful, 2 choices:
 * 1. If we expect some replies: Apply a backoff before the next transmission
 * 2. If we do not expect a reply: Remove the element from the queue and move on to the next one.
 *
 * If the transmission was not successful:
 * 1. If it was a fastrack qos: requeue without fasttrack
 * 2. Else just give up for this element, remove it from the queue.
 */
static void
  zwave_tx_message_transmission_completed_step(zwave_tx_session_id_t session_id)
{
  zwave_tx_queue_element_t completed_element = {};
  if (SL_STATUS_OK != tx_queue.get_by_id(&completed_element, session_id)) {
    // Item was already removed from the queue.(e.g due to processing the next frame)
    state = ZWAVE_TX_STATE_IDLE;
    zwave_tx_process_check_queue();
    return;
  }

  // Did the element fail and need to be requeued ? (fasttrack)
  if ((false == IS_TRANSMISSION_SUCCESSFUL(completed_element.send_data_status))
      && (completed_element.zwave_tx_options.fasttrack == true)
      && (!zwave_tx_process_queue_flush_is_ongoing())) {
    sl_log_debug(LOG_TAG,
                 "Fastrack transmit attempt failed (status = %d). "
                 "Requeueing element %p\n",
                 completed_element.send_data_status,
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
      && (!zwave_tx_process_queue_flush_is_ongoing())
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
  if (IS_TRANSMISSION_SUCCESSFUL(completed_element.send_data_status)
      && completed_element.zwave_tx_options.number_of_responses > 0
      && completed_element.connection_info.remote.is_multicast == false
      && (!zwave_tx_process_queue_flush_is_ongoing())
      && !IS_BROADCAST_NODE_ID(
        completed_element.connection_info.remote.node_id)) {
    clock_time_t backoff_time
      = completed_element.zwave_tx_options.number_of_responses
        * (completed_element.transmission_time + CLOCK_SECOND);
    sl_log_debug(LOG_TAG,
                 "Starting Z-Wave TX backoff for frame sent to NodeID %d, "
                 "backoff time: %u ms.\n",
                 completed_element.connection_info.remote.node_id,
                 backoff_time);
    state                 = ZWAVE_TX_STATE_BACKOFF;
    current_tx_session_id = session_id;
    // The backoff time should be transmission_time + 1 second.
    // Details are available in the Role Type specification, section "Node interview and response timeouts"
    etimer_set(&backoff_timer, backoff_time);
    // Here we will just return without removing the element from the queue
    // just yet, so that we can match it with a response if we receive one.
    return;
  }

  // Remove from the queue and make the correct state transision.
  zwave_tx_finalize_element(completed_element.zwave_tx_session_id);

  // Check if there is more work to do
  zwave_tx_process_check_queue();
}

static void zwave_tx_resume_from_backoff_step()
{
  if (state == ZWAVE_TX_STATE_BACKOFF) {
    // Ensure that the backoff timer is not running anymore
    etimer_stop(&backoff_timer);
    // Remove from the queue and make the correct state transision.
    zwave_tx_finalize_element(current_tx_session_id);
    // Check if there is more work to do
    zwave_tx_process_check_queue();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Functions shared among the component
////////////////////////////////////////////////////////////////////////////////
bool zwave_tx_process_queue_flush_is_ongoing()
{
  return queue_flush_ongoing;
}

void zwave_tx_process_open_tx_queue()
{
  if (queue_flush_ongoing == true) {
    sl_log_info(LOG_TAG, "Re-opening the Z-Wave Tx Queue for new frames.");
    queue_flush_ongoing = false;
  }
}

void zwave_tx_process_inspect_received_frame(zwave_node_id_t node_id)
{
  if (state == ZWAVE_TX_STATE_IDLE) {
    return;
  }

  zwave_tx_queue_element_t current_element;
  if (SL_STATUS_OK
      != tx_queue.get_by_id(&current_element, current_tx_session_id)) {
    return;
  }

  // We only match the destination NodeID, not the payload.
  if (node_id == current_element.connection_info.remote.node_id) {
    // Is that the last or do we need more replies?
    if (current_element.zwave_tx_options.number_of_responses <= 1) {
      //We got the last needed reply:
      sl_log_debug(LOG_TAG,
                   "Received all expected replies from NodeID "
                   "%d for frame id=%p.\n",
                   node_id,
                   current_element.zwave_tx_session_id);
      zwave_tx_resume_from_backoff_step();
    }

    // Decrement the number of expected responses and wait for the next one.
    tx_queue.decrement_expected_responses(current_tx_session_id);
  }
}

sl_status_t
  zwave_tx_process_abort_transmission(zwave_tx_session_id_t session_id)
{
  // Then check if it in the queue
  if (!tx_queue.contains(session_id)) {
    return SL_STATUS_NOT_FOUND;
  }

  // Ensure to remove fasttrack requeueing.
  tx_queue.disable_fasttack(session_id);

  // Ensure we are not stuck in a back-off for that frame
  if ((current_tx_session_id == session_id)
      && (state == ZWAVE_TX_STATE_BACKOFF)) {
    zwave_tx_resume_from_backoff_step();
    return SL_STATUS_OK;
  }

  // If we are idle at that point, just remove the element from the queue.
  // Pretending it failed
  if (state == ZWAVE_TX_STATE_IDLE) {
    current_tx_session_id = session_id;
    zwave_tx_drop_unsent_current_message();
    return SL_STATUS_OK;
  }

  // If we are transmitting, abort by asking the transports to abort.
  sl_status_t abort_status
    = zwave_controller_transport_abort_send_data(session_id);

  if (abort_status != SL_STATUS_OK) {
    // If the transport cannot abort, or are taking care of another frame,
    // mark it as sent/failed, so it just gets removed from the queue when
    // we get to look at the element.
    abort_status = tx_queue.set_transmissions_results(session_id,
                                                      TRANSMIT_COMPLETE_FAIL,
                                                      nullptr);
    zwave_tx_process_check_queue();
  }
  return abort_status;
}

sl_status_t zwave_tx_process_flush_queue_reset_step()
{
  sl_log_info(LOG_TAG, "Reset step: Initiating flush of the Tx Queue");

  // Mark that we want to reach an empty queue.
  queue_flush_ongoing = true;
  sl_log_info(LOG_TAG, "Tx Queue will no longer accept new frames.");

  // Is it already empty?
  if (tx_queue.empty()) {
    // Tell the controller not to wait for any callback.
    sl_log_info(LOG_TAG, "Reset step: Tx Queue flush completed. (was empty)");
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  // if we are in a back-off, shortcut the back-off
  if (state == ZWAVE_TX_STATE_BACKOFF) {
    zwave_tx_resume_from_backoff_step();
    return SL_STATUS_OK;
  }

  // If transmitting, ask the transport to abort the current element
  if (state == ZWAVE_TX_STATE_TRANSMISSION_ONGOING) {
    sl_status_t abort_status
      = zwave_controller_transport_abort_send_data(current_tx_session_id);

    if (abort_status != SL_STATUS_OK) {
      // Transport have been asked to abort, but cannot do it. We help them by
      // discarding the current message
      zwave_tx_drop_unsent_current_message();
    }
  }

  // If we are idle or just aborted, wait for all the elements to be completed
  // they will all fail from now on due to queue_flush_ongoing=true.
  return SL_STATUS_OK;
}

void zwave_tx_process_check_queue()
{
  if (ZWAVE_TX_STATE_BACKOFF != state && !tx_queue.empty()) {
    process_post(&zwave_tx_process, ZWAVE_TX_SEND_NEXT_MESSAGE, nullptr);
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
      tx_queue.clear();
      zwave_tx_init();
      zwave_tx_process_open_tx_queue();
      sl_log_info(LOG_TAG, "Z-Wave Tx process initialized.\n");

    } else if (ev == PROCESS_EVENT_EXIT) {
      state = ZWAVE_TX_STATE_IDLE;
      tx_queue.clear();
      sl_log_info(LOG_TAG, "Z-Wave Tx process exited.\n");

    } else if (ev == PROCESS_EVENT_EXITED) {
      // Do not do anything with this event, just wait to go down.

    } else if (ev == ZWAVE_TX_SEND_NEXT_MESSAGE) {
      // Look at the next frame if we are not in back-off
      if (state != ZWAVE_TX_STATE_BACKOFF) {
        zwave_tx_process_send_next_message_step();
      }

    } else if (ev == ZWAVE_TX_SEND_OPERATION_COMPLETE) {
      if (state != ZWAVE_TX_STATE_BACKOFF) {
        zwave_tx_message_transmission_completed_step(
          (zwave_tx_session_id_t)data);
      }
    } else if (ev == PROCESS_EVENT_TIMER) {
      if (data == &backoff_timer) {
        sl_log_debug(LOG_TAG,
                     "Tx backoff timer expired. Did not get all expected "
                     "answer(s) for frame id=%p.\n",
                     current_tx_session_id);
        zwave_tx_resume_from_backoff_step();
      }
    } else {
      // If an event/state combination does not make sense, the
      // state machine may get stuck, so we print out a warning when
      // such an exception occurs.
      sl_log_warning(LOG_TAG,
                     "Unhandled Z-Wave Tx Event: %s while in State: %s. "
                     "data=%p\n",
                     zwave_tx_event_name(ev),
                     zwave_tx_state_name(state),
                     data);
    }

    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
