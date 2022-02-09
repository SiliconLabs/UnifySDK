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

#ifndef ZWAVE_COMMAND_CLASS_TIME_H
#define ZWAVE_COMMAND_CLASS_TIME_H

#include "sl_status.h"
#include "zwave_rx.h"

struct zwave_controller_connection_info_t;

/**
 * @brief setup for time command class
 *
 * This setup will register the time command handler
 * to the Z-Wave CC framework,
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_time_init(void);

/**
 * @brief Handles incoming time encapsulated commands
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
sl_status_t zwave_command_class_time_support_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length);

#endif  //ZWAVE_COMMAND_CLASS_TIME_H
