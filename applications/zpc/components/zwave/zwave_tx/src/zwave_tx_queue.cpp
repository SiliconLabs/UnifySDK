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
#include "zwave_tx_queue.hpp"

// Includes from other components
#include "zwave_tx_groups.h"
#include "sl_log.h"

// Generic includes
#include <string.h>  // Using memcpy
#include <cstdio>    // snprintf

#define LOG_TAG "zwave_tx_queue"

constexpr size_t DEBUG_MESSAGE_BUFFER_LENGTH = 512;

namespace
{
// Private printing variable
char message[DEBUG_MESSAGE_BUFFER_LENGTH];

bool parent_session_id_equals(const zwave_tx_queue_element_t &element,
                              const zwave_tx_session_id_t parent_session)
{
  return (element.options.transport.valid_parent_session_id)
         && (element.options.transport.parent_session_id == parent_session);
}
}  // namespace

sl_status_t zwave_tx_queue::enqueue(const zwave_tx_queue_element_t &new_element,
                                    zwave_tx_session_id_t *user_session_id)
{
  zwave_tx_queue_element_t e = new_element;

  // Assign a session ID and provide it back to the user.
  e.zwave_tx_session_id
    = (zwave_tx_session_id_t)(uintptr_t)this->zwave_tx_session_id_counter++;
  if (user_session_id != nullptr) {
    *user_session_id = e.zwave_tx_session_id;
  }

  // Ensure that everything is initialized correctly:
  if (e.options.transport.valid_parent_session_id == false) {
    e.options.transport.parent_session_id = nullptr;
  }
  // Ensure that the timing variables are correctly set:
  e.transmission_timestamp = 0;
  e.transmission_time      = 0;
  e.queue_timestamp        = clock_time();

  // Make a console message about our new frame
  this->simple_log(&e);

  if (!queue.insert(std::move(e))) {
    return SL_STATUS_FULL;
  }
  return SL_STATUS_OK;
}

sl_status_t zwave_tx_queue::pop(const zwave_tx_session_id_t session_id)
{
  if (const auto it = find(session_id); it != queue.end()) {
    queue.erase(it);
    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_FOUND;
}

zwave_tx_queue_element_t *zwave_tx_queue::first_in_queue()
{
  return queue.begin();
}

void zwave_tx_queue::clear()
{
  queue.clear();
}

bool zwave_tx_queue::empty() const
{
  return queue.empty();
}

///////////////////////////////////////////////////////////////////////////////
// Getter functions
///////////////////////////////////////////////////////////////////////////////
bool zwave_tx_queue::contains(const zwave_tx_session_id_t session_id) const
{
  return find(session_id) != queue.end();
}

sl_status_t
  zwave_tx_queue::get_by_id(zwave_tx_queue_element_t *element,
                            const zwave_tx_session_id_t session_id) const
{
  if (auto it = find(session_id); it != queue.end()) {
    memcpy(element, &(*it), sizeof(zwave_tx_queue_element_t));
    return SL_STATUS_OK;
  }

  return SL_STATUS_NOT_FOUND;
}

sl_status_t zwave_tx_queue::get_highest_priority_child(
  zwave_tx_queue_element_t *element,
  const zwave_tx_session_id_t session_id) const
{
  sl_status_t status                    = SL_STATUS_NOT_FOUND;
  const_queue_iterator highest_priority = queue.end();

  auto it = queue.begin();
  while (it != queue.end()) {
    if (parent_session_id_equals(*it, session_id)
        && (highest_priority == queue.end()
            || queue_element_qos_compare()(*it, *highest_priority))) {
      highest_priority = it;
    }
    ++it;
  }

  if (highest_priority != queue.end()) {
    memcpy(element, highest_priority, sizeof(zwave_tx_queue_element_t));
    status = SL_STATUS_OK;
  }
  return status;
}

///////////////////////////////////////////////////////////////////////////////
// Setter functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_tx_queue::set_transmissions_results(
  const zwave_tx_session_id_t session_id,
  uint8_t status,
  zwapi_tx_report_t *tx_status)
{
  if (auto it = find(session_id); it != queue.end()) {
    it->send_data_status = status;
    if (nullptr != tx_status) {
      it->send_data_tx_status = *tx_status;
    } else {
      memset(&(it->send_data_tx_status), 0, sizeof(zwapi_tx_report_t));
    }
    // Capture the transmission time here.
    it->transmission_time = clock_time() - it->transmission_timestamp;
    // Prevent 0ms transmission time, we check on this value
    // to verify that the element was sent
    if (it->transmission_time == 0) {
      it->transmission_time = 1;
    }
    // Copy this time in the tx_report, so the user component can read this data
    it->send_data_tx_status.transmit_ticks
      = (uint16_t)(it->transmission_time / 10);

    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t zwave_tx_queue::decrement_expected_responses(
  const zwave_tx_session_id_t session_id)
{
  if (auto it = find(session_id); it != queue.end()) {
    if (it->options.number_of_responses > 0) {
      it->options.number_of_responses--;

    } else if (it->options.transport.valid_parent_session_id) {
      // Check if parents frame are expecting responses
      return this->decrement_expected_responses(
        it->options.transport.parent_session_id);
    }
    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_FOUND;
}

uint8_t zwave_tx_queue::get_number_of_responses(
  const zwave_tx_session_id_t session_id)
{
  uint8_t total_number_of_responses = 0;
  zwave_tx_queue_element_t element  = {};
  if (this->get_by_id(&element, session_id) == SL_STATUS_OK) {
    total_number_of_responses += element.options.number_of_responses;
    if (element.options.transport.valid_parent_session_id == true) {
      total_number_of_responses += this->get_number_of_responses(
        element.options.transport.parent_session_id);
    }
  }

  return total_number_of_responses;
}

const uint8_t *zwave_tx_queue::get_frame(const zwave_tx_session_id_t session_id)
{
  if (auto it = find(session_id); it != queue.end()) {
    return it->data;
  }

  return nullptr;
}

uint16_t
  zwave_tx_queue::get_frame_length(const zwave_tx_session_id_t session_id)
{
  if (auto it = find(session_id); it != queue.end()) {
    return it->data_length;
  }

  return 0;
}

sl_status_t zwave_tx_queue::set_transmission_timestamp(
  const zwave_tx_session_id_t session_id)
{
  auto it = find(session_id);
  if (it != queue.end()) {
    it->transmission_timestamp = clock_time();
    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t zwave_tx_queue::reset_transmission_timestamp(
  const zwave_tx_session_id_t session_id)
{
  auto it = find(session_id);
  if (it != queue.end()) {
    it->transmission_timestamp = 0;
    it->transmission_time      = 0;
    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t
  zwave_tx_queue::disable_fasttack(const zwave_tx_session_id_t session_id)
{
  auto it = find(session_id);
  if (it != queue.end()) {
    it->options.fasttrack = false;
    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_FOUND;
}

bool zwave_tx_queue::zwave_tx_has_frames_for_node(const zwave_node_id_t node_id)
{
  for (auto it = queue.begin(); it != queue.end(); ++it) {
    // If singlecast, just check the NodeID
    if ((it->connection_info.remote.is_multicast == false)
        && (it->connection_info.remote.node_id == node_id)) {
      return true;
    }
    // If multicast, check if the NodeID is part of the group
    if (it->connection_info.remote.is_multicast == true) {
      zwave_nodemask_t nodes = {};
      zwave_tx_get_nodes(nodes, it->connection_info.remote.multicast_group);
      if (ZW_IS_NODE_IN_MASK(node_id, nodes)) {
        return true;
      }
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Print functions
///////////////////////////////////////////////////////////////////////////////

void zwave_tx_queue::log(bool log_messages_payload) const
{
  sl_log_debug(LOG_TAG, "Queue size: %lu\n", (unsigned long)queue.size());
  for (auto it = queue.begin(); it != queue.end(); ++it) {
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
                 it->options.number_of_responses);
    sl_log_debug(LOG_TAG,
                 "\tQoS priority: %u / fasttrack %d\n",
                 it->options.qos_priority,
                 it->options.fasttrack);
    sl_log_debug(LOG_TAG,
                 "\tDiscard timeout: %d ms\n",
                 it->options.discard_timeout_ms);
    sl_log_debug(LOG_TAG,
                 "\tParent frame: %p, parent frame valid: %d\n",
                 it->options.transport.parent_session_id,
                 it->options.transport.valid_parent_session_id);
    sl_log_debug(LOG_TAG,
                 "\tMulticast group: %p, is first follow-up: %d, send "
                 "follow-ups: %d\n",
                 it->options.transport.group_id,
                 it->options.transport.is_first_follow_up,
                 it->options.send_follow_ups);
    sl_log_debug(LOG_TAG,
                 "\tTimestamps: queued %lu - transmitted %lu - Transmission "
                 "time (ms): %lu\n",
                 it->queue_timestamp,
                 it->transmission_timestamp,
                 it->transmission_time);
    if (true == log_messages_payload) {
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
                                 bool log_frame_payload) const
{
  for (auto it = queue.begin(); it != queue.end(); ++it) {
    if (it->zwave_tx_session_id == session_id) {
      sl_log_debug(
        LOG_TAG,
        "Entry (id=%p): (address %p), Qos: %u, discard timeout: %d ms, responses: %d\
 Addresses: (NodeID:Endpoint) %d:%d -> %d:%d. Multicast = %d\
 Parent frame: %p, parent frame valid: %d\
 fasttrack: %d, Queue timestamp: %lu, transmission timestamp: %lu, transmission time (ms): %lu\n",
        it->zwave_tx_session_id,
        &(*it),
        it->options.qos_priority,
        it->options.discard_timeout_ms,
        it->options.number_of_responses,
        it->connection_info.local.node_id,
        it->connection_info.local.endpoint_id,
        it->connection_info.remote.node_id,
        it->connection_info.remote.endpoint_id,
        it->connection_info.remote.is_multicast,
        it->options.transport.parent_session_id,
        it->options.transport.valid_parent_session_id,
        it->options.fasttrack,
        it->queue_timestamp,
        it->transmission_timestamp,
        it->transmission_time);
      if (true == log_frame_payload) {
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
  uint16_t index = 0;
  index += snprintf(message + index,
                    sizeof(message) - index,
                    "Enqueuing new frame (id=%p)",
                    e->zwave_tx_session_id);

  if (e->options.transport.valid_parent_session_id == true) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      " (parent id=%p)",
                      e->options.transport.parent_session_id);
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
                    "Encapsulation %d - Payload (%d bytes) [",
                    e->connection_info.encapsulation,
                    e->data_length);

  for (uint16_t i = 0; i < e->data_length; i++) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "%02X ",
                      e->data[i]);
  }
  sl_log_debug(LOG_TAG, "%s]\n", message);
}

zwave_tx_queue::queue_iterator
  zwave_tx_queue::find(const zwave_tx_session_id_t key)
{
  // TODO: Linear search of elements is rather slow. this can easily be
  // optimized to a constant complexity without excessive more memory. e.g. by
  // implementing a lookup inside the priority queue.
  return std::find_if(queue.begin(),
                      queue.end(),
                      [&key](const zwave_tx_queue_element_t &item) {
                        return item.zwave_tx_session_id == key;
                      });
}

zwave_tx_queue::const_queue_iterator
  zwave_tx_queue::find(const zwave_tx_session_id_t key) const
{
  // TODO: Linear search of elements is rather slow. this can easily be
  // optimized to a constant complexity without excessive more memory. e.g. by
  // implementing a lookup inside the priority queue.
  return std::find_if(queue.begin(),
                      queue.end(),
                      [&key](const zwave_tx_queue_element_t &item) {
                        return item.zwave_tx_session_id == key;
                      });
}
