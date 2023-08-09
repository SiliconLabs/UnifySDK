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
 * @defgroup security_2_command_class Security 2 Command Class
 * @ingroup command_classes
 * @brief Security 2 (S2) Commmand Class handler and control functions
 *
 * This module takes care of the S2 Command Class support and control.
 *
 * It uses a subscription to the ATTRIBUTE_COMMAND_CLASS_SECURITY_2_VERSION
 * attribute type in the @ref attribute_store to find out about the nodes
 * that support Security 2.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_SECURITY_2_H
#define ZWAVE_COMMAND_CLASS_SECURITY_2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"

/**
 * @brief Writes down an S2 Supported Get Command to the frame/frame_length
 *
 * @param frame         buffer where the command is to be written
 * @param frame_length Length of the data written to the output buffer.
 *
 * @return SL_STATUS_OK always
 */
sl_status_t
  zwave_command_class_security_2_commands_supported_get(uint8_t *frame,
                                                        uint16_t *frame_length);

/**
 * @brief Intitialize the security 2 command class handler
 *
 * @return Always true
 */
sl_status_t zwave_command_class_security_2_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_SECURITY_2_H
/** @} end zwave_command_class_security_2 */
