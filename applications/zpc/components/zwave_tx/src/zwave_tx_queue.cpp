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
#include "zwave_tx_queue.h"

// Includes from other components
#include "sl_log.h"

// Generic includes
#include <string.h>  // Using memcpy
#include <cstdio>    // snprintf

#define LOG_TAG "zwave_tx_queue"

sl_status_t zwave_tx_queue::enqueue(const zwave_tx_queue_element_t &new_element,
                                    zwave_tx_session_id_t *user_session_id)
{
  zwave_tx_queue_element_t e = new_element;

  // Assign a session ID and provide it back to the user.
  e.zwave_tx_session_id
    = (zwave_tx_session_id_t)(uintptr_t)this->zwave_tx_session_id_counter++;
  if (user_session_id != NULL) {
    *user_session_id = e.zwave_tx_session_id;
  }

  // Ensure that everything is initialized correctly:
  if (e.zwave_tx_options.valid_parent_session_id == false) {
    e.zwave_tx_options.parent_session_id = 0;
  }
  // Ensure that the timing variables are correctly set:
  e.transmission_timestamp = 0;
  e.transmission_time      = 0;
  e.queue_timestamp        = clock_time();

  // A bit of validation: If multicast destination, then number of expected replies must be 0.
  if (e.connection_info.remote.is_multicast == true) {
    e.zwave_tx_options.number_of_responses = 0;
  }

  // Make a console message about our new frame, only for initial frames
  if (e.zwave_tx_options.valid_parent_session_id == false) {
    this->simple_log(&e);
  }
  // Put our element in the list and keep it sorted, so highest QoS is first.
  this->insert(e);
  return SL_STATUS_OK;
}

sl_status_t zwave_tx_queue::pop(const zwave_tx_session_id_t session_id)
{
  zwave_tx_queue::iterator it;
  for (it = this->begin(); it != this->end(); ++it) {
    if (it->zwave_tx_session_id == session_id) {
      it = this->erase(it);
      return SL_STATUS_OK;
    }
  }

  return SL_STATUS_NOT_FOUND;
}

///////////////////////////////////////////////////////////////////////////////
// Getter functions
///////////////////////////////////////////////////////////////////////////////

sl_status_t
  zwave_tx_queue::get_by_id(zwave_tx_queue_element_t *element,
                            const zwave_tx_session_id_t session_id) const
{
  zwave_tx_queue::iterator it;
  for (it = this->begin(); it != this->end(); ++it) {
    if (it->zwave_tx_session_id == session_id) {
      memcpy(element, &(*it), sizeof(zwave_tx_queue_element_t));
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t
  zwave_tx_queue::get_child_by_id(zwave_tx_queue_element_t *element,
                                  const zwave_tx_session_id_t session_id,
                                  uint32_t child_index) const
{
  zwave_tx_queue::iterator it;
  uint32_t child_count = 0;
  for (it = this->begin(); it != this->end(); ++it) {
    if ((it->zwave_tx_options.valid_parent_session_id)
        && (it->zwave_tx_options.parent_session_id == session_id)) {
      if (child_count == child_index) {
        memcpy(element, &(*it), sizeof(zwave_tx_queue_element_t));
        return SL_STATUS_OK;
      }
      child_count += 1;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

///////////////////////////////////////////////////////////////////////////////
// Setter functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_tx_queue::set_transmissions_results(
  const zwave_tx_session_id_t session_id,
  uint8_t status,
  zwapi_tx_report_t *tx_status)
{
  zwave_tx_queue::iterator it;
  for (it = this->begin(); it != this->end(); ++it) {
    if (it->zwave_tx_session_id == session_id) {
      zwave_tx_queue_element_t e = *it;
      e.send_data_status         = status;
      if (nullptr != tx_status) {
        e.send_data_tx_status = *tx_status;
      } else {
        memset(&(e.send_data_tx_status), 0, sizeof(zwapi_tx_report_t));
      }
      // Capture the transmission time here.
      e.transmission_time = clock_time() - e.transmission_timestamp;
      // Copy this time in the tx_report, so the user component can read this data
      e.send_data_tx_status.wTransmitTicks
        = (uint16_t)(e.transmission_time / 10);
      // Remove the old element and re-insert the modified version of it.
      this->insert(it, e);  // Hint the position of the element to remove.
      this->erase(it);      // Remove the old element
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t zwave_tx_queue::decrement_expected_responses(
  const zwave_tx_session_id_t session_id)
{
  zwave_tx_queue::iterator it;
  for (it = this->begin(); it != this->end(); ++it) {
    if (it->zwave_tx_session_id == session_id) {
      if (it->zwave_tx_options.number_of_responses > 0) {
        zwave_tx_queue_element_t e = *it;
        e.zwave_tx_options.number_of_responses--;
        // Remove the old element and re-insert the modified version of it.
        this->insert(it, e);  // Hint the position of the element to remove.
        this->erase(it);      // Remove the old element
      }
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t
  zwave_tx_queue::set_expected_responses(const zwave_tx_session_id_t session_id,
                                         uint8_t number_of_responses)
{
  zwave_tx_queue::iterator it;
  for (it = this->begin(); it != this->end(); ++it) {
    if (it->zwave_tx_session_id == session_id) {
      zwave_tx_queue_element_t e             = *it;
      e.zwave_tx_options.number_of_responses = number_of_responses;
      // Remove the old element and re-insert the modified version of it.
      this->insert(it, e);  // Hint the position of the element to remove.
      this->erase(it);      // Remove the old element
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t zwave_tx_queue::set_transmission_timestamp(
  const zwave_tx_session_id_t session_id)
{
  zwave_tx_queue::iterator it;
  for (it = this->begin(); it != this->end(); ++it) {
    if (it->zwave_tx_session_id == session_id) {
      // Make a copy of the element and modify its sending time.
      zwave_tx_queue_element_t e = *it;
      e.transmission_timestamp   = clock_time();
      // Remove the old element and re-insert the modified version of it.
      this->insert(it, e);  // Hint the position of the element to remove.
      this->erase(it);      // Remove the old element
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t zwave_tx_queue::reset_transmission_timestamp(
  const zwave_tx_session_id_t session_id)
{
  zwave_tx_queue::iterator it;
  for (it = this->begin(); it != this->end(); ++it) {
    if (it->zwave_tx_session_id == session_id) {
      // Make a copy of the element and modify its sending time.
      zwave_tx_queue_element_t e = *it;
      e.transmission_timestamp   = 0;
      e.transmission_time        = 0;
      // Remove the old element and re-insert the modified version of it.
      this->insert(it, e);  // Hint the position of the element to remove.
      this->erase(it);      // Remove the old element
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}
sl_status_t
  zwave_tx_queue::disable_fasttack(const zwave_tx_session_id_t session_id)
{
  zwave_tx_queue::iterator it;
  for (it = this->begin(); it != this->end(); ++it) {
    if (it->zwave_tx_session_id == session_id) {
      // Make a copy of the element and modify its data.
      zwave_tx_queue_element_t e   = *it;
      e.zwave_tx_options.fasttrack = false;
      // Remove the old element and re-insert the modified version of it.
      this->insert(it, e);  // Hint the position of the element to remove.
      this->erase(it);      // Remove the old element
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t zwave_tx_queue::cancel_follow_up_frames(
  const zwave_tx_session_id_t session_id)
{
  zwave_tx_queue::iterator it;
  for (it = this->begin(); it != this->end(); ++it) {
    if (it->zwave_tx_session_id == session_id) {
      // Make a copy of the element and modify its data.
      zwave_tx_queue_element_t e         = *it;
      e.zwave_tx_options.send_follow_ups = false;
      // Remove the old element and re-insert the modified version of it.
      this->insert(it, e);  // Hint the position of the element to remove.
      this->erase(it);      // Remove the old element
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

///////////////////////////////////////////////////////////////////////////////
// Print functions
///////////////////////////////////////////////////////////////////////////////

void zwave_tx_queue::log(bool log_messages_payload) const
{
  zwave_tx_queue::iterator it;
  sl_log_debug(LOG_TAG, "Queue size: %lu\n", (unsigned long)this->size());
  for (it = this->begin(); it != this->end(); ++it) {
    sl_log_debug(LOG_TAG,
                 "Entry (id=%p): (address %p)\n",
                 it->zwave_tx_session_id,
                 &(*it));
    sl_log_debug(LOG_TAG,
                 "\tCallback: %p, user pointer: %p \n",
                 it->callback_function,
                 it->user);
    sl_log_debug(
      LOG_TAG,
      "\tAddresses: (NodeID:Endpoint) %d:%d -> %d:%d, is_multicast: %d\n",
      it->connection_info.local.node_id,
      it->connection_info.local.endpoint_id,
      it->connection_info.remote.node_id,
      it->connection_info.remote.endpoint_id,
      it->connection_info.remote.is_multicast);
    sl_log_debug(LOG_TAG,
                 "\tEncapsulation: %d - Number of expected responses: %d\n",
                 it->connection_info.encapsulation,
                 it->zwave_tx_options.number_of_responses);
    sl_log_debug(LOG_TAG,
                 "\tQoS priority: %u / fasttrack %d\n",
                 it->zwave_tx_options.qos_priority,
                 it->zwave_tx_options.fasttrack);
    sl_log_debug(LOG_TAG,
                 "\tDiscard timeout: %d ms\n",
                 it->zwave_tx_options.discard_timeout_ms);
    sl_log_debug(
      LOG_TAG,
      "\tParent frame: %p, use parent options: %d, parent frame valid: %d\n",
      it->zwave_tx_options.parent_session_id,
      it->zwave_tx_options.use_parent_frame_options,
      it->zwave_tx_options.valid_parent_session_id);
    sl_log_debug(LOG_TAG,
                 "\tMulticast group: %p, is first follow-up: %d, send "
                 "follow-ups: %d\n",
                 it->zwave_tx_options.group_id,
                 it->zwave_tx_options.is_first_follow_up,
                 it->zwave_tx_options.send_follow_ups);
    sl_log_debug(LOG_TAG,
                 "\tTimestamps: queued %lu - transmitted %lu - Transmission "
                 "time (ms): %lu\n",
                 it->queue_timestamp,
                 it->transmission_timestamp,
                 it->transmission_time);
    if (true == log_messages_payload) {
      char message[1000];
      uint16_t index = 0;

      index += snprintf(message + index,
                        sizeof(message) - index,
                        "Frame payload (hex): ");

      for (uint16_t i = 0; i < it->data_length; i++) {
        index += snprintf(message + index,
                          sizeof(message) - index,
                          "%02X ",
                          it->data[i]);
      }
      sl_log_debug(LOG_TAG, "\t%s\n", message);
    }
  }
}

void zwave_tx_queue::log_element(const zwave_tx_session_id_t session_id,
                                 bool log_messages_payload) const
{
  zwave_tx_queue::iterator it;
  for (it = this->begin(); it != this->end(); ++it) {
    if (it->zwave_tx_session_id == session_id) {
      sl_log_debug(
        LOG_TAG,
        "Entry (id=%p): (address %p), Qos: %u, discard timeout: %d ms, responses: %d\
 Addresses: (NodeID:Endpoint) %d:%d -> %d:%d. Multicast = %d\
 Parent frame: %p, use parent options: %d, parent frame valid: %d\
 fasttrack: %d, Queue timestamp: %lu, transmission timestamp: %lu, transmission time (ms): %lu\n",
        it->zwave_tx_session_id,
        &(*it),
        it->zwave_tx_options.qos_priority,
        it->zwave_tx_options.discard_timeout_ms,
        it->zwave_tx_options.number_of_responses,
        it->connection_info.local.node_id,
        it->connection_info.local.endpoint_id,
        it->connection_info.remote.node_id,
        it->connection_info.remote.endpoint_id,
        it->connection_info.remote.is_multicast,
        it->zwave_tx_options.parent_session_id,
        it->zwave_tx_options.use_parent_frame_options,
        it->zwave_tx_options.valid_parent_session_id,
        it->zwave_tx_options.fasttrack,
        it->queue_timestamp,
        it->transmission_timestamp,
        it->transmission_time);
      if (true == log_messages_payload) {
        char message[1000];
        uint16_t index = 0;

        index += snprintf(message + index,
                          sizeof(message) - index,
                          "Frame payload (hex): ");

        for (uint16_t i = 0; i < it->data_length; i++) {
          index += snprintf(message + index,
                            sizeof(message) - index,
                            "%02X ",
                            it->data[i]);
        }
        sl_log_debug(LOG_TAG, "%s\n", message);
      }
      return;
    }
  }
  sl_log_warning(LOG_TAG, "Element (id=%p) is not in the queue\n", session_id);
}

void zwave_tx_queue::simple_log(zwave_tx_queue_element_t *e) const
{
  char message[1000];
  uint16_t index = 0;

  index += snprintf(message + index,
                    sizeof(message) - index,
                    "Enqueuing new frame (id=%p)",
                    e->zwave_tx_session_id);

  if (e->zwave_tx_options.valid_parent_session_id == true) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      " (parent id=%p)",
                      e->zwave_tx_options.parent_session_id);
  }

  // Source address
  index += snprintf(message + index,
                    sizeof(message) - index,
                    " - %d:%d -> ",
                    e->connection_info.local.node_id,
                    e->connection_info.local.endpoint_id);

  // Destination
  if (e->connection_info.remote.is_multicast == false) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      " %d:%d - ",
                      e->connection_info.remote.node_id,
                      e->connection_info.remote.endpoint_id);
  } else {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "Group ID %d (endpoint=%d) - ",
                      e->connection_info.remote.multicast_group,
                      e->connection_info.remote.endpoint_id);
  }

  // Encapsulation & payload
  index += snprintf(message + index,
                    sizeof(message) - index,
                    "Encapsulation %d - Payload [",
                    e->connection_info.encapsulation);

  for (uint16_t i = 0; i < e->data_length; i++) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "%02X ",
                      e->data[i]);
  }
  sl_log_debug(LOG_TAG, "%s]\n", message);
}
