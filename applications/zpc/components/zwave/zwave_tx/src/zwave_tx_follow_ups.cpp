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
#include "zwave_tx_process.h"
#include "zwave_tx_queue.h"
#include "zwave_tx_follow_ups.h"
#include "zwave_controller_internal.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "zwave_tx_groups.h"

#include "sl_log.h"
constexpr char LOG_TAG[] = "zwave_tx_follow_ups";

// QoS Offset for the first follow up frame
constexpr uint32_t SINGLECAST_FIRST_FOLLOW_UP_QOS_BOOST = 3;

// Shared variables from the Z-Wave TX Process
extern zwave_tx_queue tx_queue;

// Static variables, used to re-queue a frame.
namespace tx_follow_ups
{
uint8_t frame_length = 0;
uint8_t frame[ZWAVE_MAX_FRAME_SIZE];
}  // namespace tx_follow_ups

sl_status_t
  zwave_tx_enqueue_follow_ups(zwave_tx_session_id_t multicast_frame_tx_id)
{
  zwave_tx_queue_element_t e;
  if (SL_STATUS_OK != tx_queue.get_by_id(&e, multicast_frame_tx_id)) {
    return SL_STATUS_NOT_FOUND;
  }

  // Sanity check, should not happen, but better safe than sorry
  if (e.connection_info.remote.is_multicast == false) {
    sl_log_error(
      LOG_TAG,
      "Ignoring request to send follow-ups from a singlecast frame.");
    return SL_STATUS_FAIL;
  }

  // Copy the frame data
  if (e.data_length > sizeof(tx_follow_ups::frame)) {
    return SL_STATUS_WOULD_OVERFLOW;
  }
  memcpy(tx_follow_ups::frame, e.data, e.data_length);
  tx_follow_ups::frame_length = (uint8_t)e.data_length;

  // Find all the nodes in the group:
  zwave_nodemask_t node_list = {};
  memset(node_list, 0, sizeof(zwave_nodemask_t));
  if (SL_STATUS_OK
      != zwave_tx_get_nodes(node_list,
                            e.connection_info.remote.multicast_group)) {
    sl_log_error(LOG_TAG,
                 "Cannot retrieve Group ID %d node list. "
                 "Aborting singlecast follow-up frames for id=%p",
                 e.connection_info.remote.multicast_group,
                 multicast_frame_tx_id);
    return SL_STATUS_FAIL;
  }

  // Prepare all the data common to all singlecast follow-up frames.
  zwave_controller_connection_info_t info = {};
  zwave_tx_options_t options              = {};

  info.local.is_multicast  = false;
  info.local.node_id       = 0;
  info.local.endpoint_id   = 0;
  info.encapsulation       = e.connection_info.encapsulation;
  info.remote.is_multicast = false;
  info.remote.endpoint_id  = e.connection_info.remote.endpoint_id;

  options.parent_session_id       = e.zwave_tx_session_id;
  options.valid_parent_session_id = true;
  options.discard_timeout_ms      = e.zwave_tx_options.discard_timeout_ms;
  // The QoS Priority will be adjusted for each node.
  options.qos_priority        = e.zwave_tx_options.qos_priority;
  options.number_of_responses = e.zwave_tx_options.number_of_responses;
  options.is_test_frame       = false;
  options.rf_power            = NORMAL_POWER;
  options.is_first_follow_up  = true;
  options.send_follow_ups     = false;
  options.fasttrack           = true;
  options.group_id            = e.connection_info.remote.multicast_group;

  for (zwave_node_id_t n = ZW_MIN_NODE_ID; n <= ZW_LR_MAX_NODE_ID; n++) {
    if (false == ZW_IS_NODE_IN_MASK(n, node_list)) {
      continue;
    }

    options.qos_priority = e.zwave_tx_options.qos_priority;
    // Remove the QoS boost for the first Singlecast follow-up frame, so others are sent afterwards
    if (options.is_first_follow_up == true) {
      options.qos_priority += SINGLECAST_FIRST_FOLLOW_UP_QOS_BOOST;
    }

    info.remote.node_id = n;
    zwave_tx_send_data(&info,
                       tx_follow_ups::frame_length,
                       tx_follow_ups::frame,
                       &options,
                       nullptr,
                       nullptr,
                       nullptr);

    // After enqueuing the first follow-up, it is not the first follow up anymore
    options.is_first_follow_up = false;
  }

  // Finally, mark the original multicast message as "send_follow_ups" = false
  // so we do not queue more when we are done with this bunch of follow-ups.
  tx_queue.cancel_follow_up_frames(multicast_frame_tx_id);

  return SL_STATUS_OK;
}
