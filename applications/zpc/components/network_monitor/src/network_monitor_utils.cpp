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
// Includes from this component
#include "network_monitor_utils.h"

// Unify components
#include "attribute.hpp"
#include "attribute_resolver.h"
#include "sys/clock.h"

// ZPC Components
#include "attribute_store_defined_attribute_types.h"

bool network_monitor_is_node_asleep_due_to_inactivity(
  attribute_store_node_t node_id_node, int32_t inactive_time)
{
  // If the node is paused for resolution, we are not in a Wake Up session anyway.
  // It can be that node just sent us in the last 10 seconds, but no Wake Up Notification.
  if (true == is_node_or_parent_paused(node_id_node)) {
    return true;
  }

  clock_time_t current_time = clock_seconds();
  clock_time_t last_rx_tx   = 0;
  attribute_store_node_t last_tx_rx_node
    = attribute_store_get_first_child_by_type(
      node_id_node,
      ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP);
  attribute_store_get_reported(last_tx_rx_node,
                               &last_rx_tx,
                               sizeof(last_rx_tx));
  return ((int32_t)(current_time - last_rx_tx) > inactive_time);
}

/**
 * @brief Gets the Z-Wave operating mode from the Attribute Store Node for the NodeID
 *
 * @param node_id_node        NodeID node in the attribute store.
 * @returns zwave_operating_mode_t
 */
zwave_operating_mode_t
  network_monitor_get_operating_mode(attribute_store_node_t node_id_node)
{
  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));
  return zwave_get_operating_mode(node_id);
}