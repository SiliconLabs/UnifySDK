/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zwave_network_management_return_route_queue.h"
#include "zwave_network_management_callbacks.h"
#include "zwave_network_management_process.h"

// Private queue of elements to assign
static assign_return_route_t queue[ASSIGN_RETURN_ROUTE_QUEUE_SIZE] = {};

// Values to set in un-assigned entries for the return route queue
#define UNASSIGNED_ENTRY 0

#include "sl_log.h"
#define LOG_TAG "zwave_network_management_callbacks"

/**
 * @brief Resets the value of a queue entry.
 *
 * @param entry Pointer to an entry in the assign return route queue
 */
static void clear_queue_entry(assign_return_route_t *entry)
{
  // Erase the entry
  entry->node_id             = UNASSIGNED_ENTRY;
  entry->destination_node_id = UNASSIGNED_ENTRY;
  entry->state               = ROUTE_STATE_UNASSIGNED_ENTRY;
  entry->reuse_timestamp     = 0;
}

void zwave_network_management_return_route_clear_queue()
{
  for (uint8_t i = 0; i < ASSIGN_RETURN_ROUTE_QUEUE_SIZE; i++) {
    clear_queue_entry(&queue[i]);
  }
}

bool we_have_return_routes_to_assign(zwave_node_id_t node_id)
{
  for (uint8_t i = 0; i < ASSIGN_RETURN_ROUTE_QUEUE_SIZE; i++) {
    if ((queue[i].node_id == node_id)
        && (queue[i].state != ROUTE_STATE_ESTABLISHED)) {
      return true;
    }
  }
  return false;
}

sl_status_t zwave_network_management_return_route_add_to_queue(
  const assign_return_route_t *request)
{
  for (uint8_t i = 0; i < ASSIGN_RETURN_ROUTE_QUEUE_SIZE; i++) {
    // Clear entry if it can be reused now.
    if ((queue[i].state == ROUTE_STATE_ESTABLISHED)
        && (queue[i].reuse_timestamp < clock_time())) {
      sl_log_debug(LOG_TAG,
                   "Clearing Return Route queue entry %i for reuse.",
                   i);
      clear_queue_entry(&queue[i]);
    }
    // Check if we already have this return route in the queue (in progress or not)
    if ((queue[i].node_id == request->node_id)
        && (queue[i].destination_node_id == request->destination_node_id)) {
      sl_log_debug(
        LOG_TAG,
        "Assign Return Route request for NodeID %i towards NodeID %i "
        "already in progress or recently done. Ignoring new request.",
        request->node_id,
        request->destination_node_id);
      return SL_STATUS_OK;
    }
  }

  // If not, find the first available slot:
  for (uint8_t i = 0; i < ASSIGN_RETURN_ROUTE_QUEUE_SIZE; i++) {
    if (queue[i].node_id == UNASSIGNED_ENTRY) {
      queue[i].node_id             = request->node_id;
      queue[i].destination_node_id = request->destination_node_id;
      queue[i].state               = ROUTE_STATE_NOT_ESTABLISHED;
      process_post(&zwave_network_management_process,
                   NM_EV_ASSIGN_RETURN_ROUTE_START,
                   NULL);
      return SL_STATUS_OK;
    }
  }

  // No luck queue is full, we just return fail.
  sl_log_debug(LOG_TAG,
               "Assign Return Route request queue is full. "
               "Ignoring request for NodeID %i towards NodeID %i.",
               request->node_id,
               request->destination_node_id);
  return SL_STATUS_FAIL;
}

sl_status_t zwave_network_management_return_route_assign_next()
{
  // Do not assign the next if one is in progress:
  for (uint8_t i = 0; i < ASSIGN_RETURN_ROUTE_QUEUE_SIZE; i++) {
    if (queue[i].state == ROUTE_STATE_IN_PROGRESS) {
      return SL_STATUS_IN_PROGRESS;
    }
  }

  // None in progress, find the first entry and send it to the Z-Wave API
  for (uint8_t i = 0; i < ASSIGN_RETURN_ROUTE_QUEUE_SIZE; i++) {
    if (queue[i].state == ROUTE_STATE_NOT_ESTABLISHED) {
      sl_log_debug(LOG_TAG,
                   "Establishing return route from NodeIDs %d -> %d",
                   queue[i].node_id,
                   queue[i].destination_node_id);
      sl_status_t status
        = zwapi_assign_return_route(queue[i].node_id,
                                    queue[i].destination_node_id,
                                    &on_assign_return_route_complete);
      if (status == SL_STATUS_OK) {
        queue[i].state = ROUTE_STATE_IN_PROGRESS;
        return SL_STATUS_IN_PROGRESS;
      } else {
        queue[i].state           = ROUTE_STATE_ESTABLISHED;
        queue[i].reuse_timestamp = clock_time() + ASSIGN_RETURN_ROUTE_COOLDOWN;
      }
    }
  }

  // We are not assigning anything anymore
  return SL_STATUS_IDLE;
}

void on_assign_return_route_complete(uint8_t status)
{
  // We do not really care about the status.
  (void)status;

  // Mark the route as completed, un-assign the entry that was in progress
  // It should be only 1 ideally
  for (uint8_t i = 0; i < ASSIGN_RETURN_ROUTE_QUEUE_SIZE; i++) {
    if (queue[i].state == ROUTE_STATE_IN_PROGRESS) {
      queue[i].state           = ROUTE_STATE_ESTABLISHED;
      queue[i].reuse_timestamp = clock_time() + ASSIGN_RETURN_ROUTE_COOLDOWN;
    }
  }

  // If we have more to do, do it immediately.
  // Else tell the NM state machine that we are done.
  sl_status_t assign_next_status
    = zwave_network_management_return_route_assign_next();
  if (assign_next_status != SL_STATUS_IN_PROGRESS) {
    process_post(&zwave_network_management_process,
                 NM_EV_ASSIGN_RETURN_ROUTE_COMPLETED,
                 NULL);
  }
}