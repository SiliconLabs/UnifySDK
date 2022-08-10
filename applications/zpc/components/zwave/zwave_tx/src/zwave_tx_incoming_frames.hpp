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

/**
 * @defgroup zwave_tx_incoming_frames Z-Wave Tx Incoming frames
 * @ingroup zwave_tx
 * @brief Small module that helps keeping track of incoming frames
 *        from remote NodeIDs.
 *
 * @{
 */

#ifndef ZWAVE_TX_INCOMING_FRAMES_HPP
#define ZWAVE_TX_INCOMING_FRAMES_HPP

// Includes from this component
#include "zwave_tx.h"
#include "priority_queue.hpp"

// Includes from other components
#include "zwave_controller_types.h"

// Type definition for the number of frames to expect
using number_of_frames_t = uint8_t;

/**
 * @brief Keeps track of NodeIDs and their expected spontaneous frames
 *
 */
using zwave_tx_incoming_frame_item_t = struct zwave_tx_incoming_frame_item {
  // Remote NodeID that will send us some frames in the near future
  zwave_node_id_t node_id;
  // Number of frames that we expect to receive from NodeID
  number_of_frames_t incoming_frames;
};

/**
 * @brief Comparion function for zwave_tx_incoming_frame_item_t objects
 */
struct incoming_frame_item_compare {
  bool operator()(const zwave_tx_incoming_frame_item_t &lhs,
                  const zwave_tx_incoming_frame_item_t &rhs) const
  {
    return lhs.node_id > rhs.node_id;
  }
};

// Class definition that keeps track of incoming frames
class zwave_tx_incoming_frames
{
  private:
  // List of frames that we expect from NodeIDs.
  priority_queue<zwave_tx_incoming_frame_item_t,
                 ZWAVE_TX_INCOMING_FRAMES_BUFFER_SIZE,
                 incoming_frame_item_compare>
    frame_list;

  /**
   * @brief Helper function that inserts a NodeID / Frame pair in the frame
   * list
   *
   * It is expected to verify manually that the NodeID does not exist in the
   * frame list before invoking this function.
   *
   * @param node_id           NodeID to insert
   * @param incoming_frames   Number of frames to set for the NodeID.
   * @returns SL_STATUS_OK on success, SL_STATUS_FAIL otherwise.
   */
  sl_status_t insert(zwave_node_id_t node_id,
                     number_of_frames_t incoming_frames)
  {
    zwave_tx_incoming_frame_item_t item = {};
    item.node_id                        = node_id;
    item.incoming_frames                = incoming_frames;
    return (frame_list.insert(std::move(item)) == true ? SL_STATUS_OK
                                                       : SL_STATUS_FAIL);
  }

  public:
  /**
   * @brief Sets the number of expected frames from a NodeID
   *
   * @param node_id           NodeID to set the frames for
   * @param incoming_frames   Number of frames to set for the NodeID.
   * @returns SL_STATUS_OK on success, SL_STATUS_FAIL on failure,
   *          SL_STATUS_NOT_FOUND if nothing is to be done.
   */
  sl_status_t set_frames(zwave_node_id_t node_id,
                         number_of_frames_t incoming_frames)
  {
    for (auto it = frame_list.begin(); it != frame_list.end(); ++it) {
      if (it->node_id == node_id) {
        // Decrement
        if (incoming_frames == 0) {
          frame_list.erase(it);
          return SL_STATUS_OK;
        } else {
          it->incoming_frames = incoming_frames;
          return SL_STATUS_OK;
        }
      }
    }

    // We don't know about this NodeID. Insert it.
    if (incoming_frames > 0) {
      return insert(node_id, incoming_frames);
    }

    return SL_STATUS_NOT_FOUND;
  }

  /**
   * @brief Decrements the number of expected frames from a NodeID
   *
   * @param node_id           NodeID to decrement the frames from
   * @returns SL_STATUS_OK on success
   *          SL_STATUS_NOT_FOUND if nothing is to be done.
   */
  sl_status_t decrement_frames(zwave_node_id_t node_id)
  {
    for (auto it = frame_list.begin(); it != frame_list.end(); ++it) {
      if (it->node_id == node_id) {
        // Decrement
        if (it->incoming_frames <= 1) {
          frame_list.erase(it);
          return SL_STATUS_OK;
        } else {
          it->incoming_frames -= 1;
          return SL_STATUS_OK;
        }
      }
    }
    return SL_STATUS_NOT_FOUND;
  }

  /**
   * @brief Gets the number of expected frames from a NodeID
   *
   * @param node_id  NodeID for which we expect to receive some frames
   * @returns 0 if no frame is expected, else the number of frames
   */
  number_of_frames_t get_frames(zwave_node_id_t node_id)
  {
    for (auto it = frame_list.begin(); it != frame_list.end(); ++it) {
      if (it->node_id == node_id) {
        return it->incoming_frames;
      }
    }

    // Not found, return no frame.
    return 0;
  }

  /**
   * @brief Clears the entire list of expected frames
   */
  void clear()
  {
    frame_list.clear();
  }

  /**
   * @brief Checks if we have any expected frame at all
   * @returns true if there are expected frames, false otherwise
   */
  bool empty() const
  {
    return frame_list.empty();
  }
};

#endif  //ZWAVE_TX_INCOMING_FRAMES_HPP
/** @} end zwave_tx_incoming_frames */
