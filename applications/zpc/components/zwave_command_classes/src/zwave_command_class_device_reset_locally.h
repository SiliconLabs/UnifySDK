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
 * @defgroup device_reset_locally_command_class Device Reset Locally Command Class
 * @ingroup command_classes
 * @brief Device Reset Locally Command Class handlers and init functions
 *
 * 
 * @{
*/


#ifndef ZWAVE_COMMAND_CLASS_DEVICE_RESET_LOCALLY_H
#define ZWAVE_COMMAND_CLASS_DEVICE_RESET_LOCALLY_H

#include "sl_status.h"
#include "zwave_utils.h"
#include "zwave_rx.h"
#include "zwave_controller_connection_info.h"

// Z-Wave nodes issue Device Reset Locally command before they reset their
// HomeID and NodeID. Due to that, ZPC shall wait at-least 1 second
// before triggering removal of the node.
#define DEVICE_RESET_LOCALLY_REMOVE_TRIGGER_TIMEOUT_DEFAULT 1000
#define DEVICE_RESET_LOCALLY_REMOVE_RETRIES_MAX             3

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handles incoming Device Reset Locally Command
 *
 * @param connection_info Info about the connection properties of this frame.
 * @param frame_data The data payload of this frame.
 * @param frame_length The length of this frame.
 * @returns sl_status_t representing the outcome of receiving the frame
 *          In case where the command is controlled, it should be set to SL_STATUS_OK,
 *          even when encountering some parsing errors
 *          In case where the command is supported, it should be set to the @ref sl_status_t
 *          corresponding to the correct time Status code. Refer to @ref zwave_command_handler_t
 */
sl_status_t zwave_command_class_device_reset_locally_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length);

/**
 * @brief Intitialize the Device Reset Locally command class control APIs
 *
 * This setup will register the device reset locally notification command handler to the Z-Wave CC framework.
 *
 * @return Always true
 */
sl_status_t zwave_command_class_device_reset_locally_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_DEVICE_RESET_LOCALLY_H
/** @} end zwave_command_class_device_reset_locally */
