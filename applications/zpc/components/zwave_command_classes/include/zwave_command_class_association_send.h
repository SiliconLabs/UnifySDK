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

/**
 * @defgroup zwave_command_class_association_send Association Group transport
 * @ingroup association_command_class
 * @brief Utility that deliver frames to Association Group destinations.
 *
 * This module provides an API to send frames to Association Group destinations
 * Destinations can both be NodeID or Endpoint Associations
 * (see @ref multi_channel_association_command_class
 * and @ref zwave_command_class_association_internals for details.)
 *
 * Frames command classes and commands will be verified against what is
 * registered on each group using @ref zwave_command_class_agi_add_group_commands()
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_ASSOCIATION_SEND_H
#define ZWAVE_COMMAND_CLASS_ASSOCIATION_SEND_H

#include "zwave_command_class_association_types.h"

#include "sl_status.h"
#include "zwave_tx.h"
#include "zwave_utils.h"  // for zwave_node_id_t

/**
 * @brief Callback function for zwave_association_send_to_group,
 *        indicating the result of the operation
 *
 * This function is called when all queued elements in the TX Queue have been
 * transmitted (or attempted to be transmitted) and indicates the status of the
 * operation.
 *
 * @param status  Indicates how the transmission operation was completed.
 *                SL_STATUS_OK for all transmissions were successful
 *                SL_STATUS_FAIL if 1 or more transmissions failed
 * @param user    User pointer provided in \ref zwave_association_send_to_group()
 */
typedef void (*on_group_send_complete_t)(sl_status_t status, void *user);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sends a command to an Association Group.
 *
 * @param group_id        Association Group to which the
 * @param frame           Pointer to the frame data. The Command Class and Command
 *                        sent to a group MUST have been registered with
 *                        @ref zwave_command_class_association_add_group_commands
 * @param frame_size      Size of the payload carried by the Frame pointer.
 * @param supporting      Set to true if the frame we send is supported
 *                        (i.e sent at our highest security level).
 *                        False for a controlling frame.
 * @param tx_options      Pointer to zwave_tx_options for the transmission.
 * @param excluded_node   NodeID to exclude from the group (e.g. if lifeline
 *                        true status is to be reported to everyone but the
 *                        NodeID that triggered the change.) Set to 0 for
 *                        no exclusion.
 * @param callback        Callback to invoke when all transmissions for all
 *                        destinations are completed/failed.
 * @param user            User pointer for the callback.
 *
 * Note that no abort function is provided. Set the discard timeout to a low
 * value in the tx_options if you do not want a transmission to take too long.
 *
 * @returns SL_STATUS_OK if the frame(s) were queued and callback will be
 *          called when all transmissions are done. SL_STATUS_NOT_FOUND if the
 *          association group is empty or no transmission is to be done
 *          SL_STATUS_FAIL in case of error.
 */
sl_status_t
  zwave_association_send_to_group(association_group_id_t group_id,
                                  uint16_t frame_size,
                                  const uint8_t *frame,
                                  bool supporting,
                                  const zwave_tx_options_t *tx_options,
                                  zwave_node_id_t excluded_node,
                                  on_group_send_complete_t callback,
                                  void *user);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_ASSOCIATION_SEND_H
        /** @} end zwave_command_class_association_send */