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

/**
 * @defgroup security_0_command_class Security 0 Command Class
 * @ingroup command_classes
 * @brief Security 0 (S0) Commmand Class handler and control functions
 *
 * This module takes care of the S0 Command Class support and control.
 *
 * TODO: Fill me up when implementing S0
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_SECURITY_0_H
#define ZWAVE_COMMAND_CLASS_SECURITY_0_H

#include "zwave_rx.h"
#include "sl_status.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Writes down an S0 Supported Get Command to the frame/frame_length
 *
 * @param frame         buffer where the command is to be written
 * @param frame_length Length of the data written to the output buffer.
 *
 * @return SL_STATUS_OK always
 */
sl_status_t
  zwave_command_class_security_0_commands_supported_get(uint8_t *frame,
                                                        uint16_t *frame_length);

/**
 * @brief Intitialize the Security 0 Command Class handler
 *
 * @return Always true
 */
sl_status_t zwave_command_class_security_0_init();

sl_status_t zwave_command_class_security_0_support_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length);

sl_status_t zwave_command_class_security_0_control_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_SECURITY_0_H
/** @} end security_0_command_class */
