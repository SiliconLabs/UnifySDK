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
 * @defgroup zwave_command_class_switch_color Switch Color Command Class
 * @ingroup command_classes
 * @brief Switch Color Command Class handlers and control functions
 *
 * This module implement some of the functions to control the Color Switch
 * Command Class
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_SWITCH_COLOR_H
#define ZWAVE_COMMAND_CLASS_SWITCH_COLOR_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function initialize the Color Switch Command Class handler
 *
 * @return SL_STATUS_OK on success, any other error code for an error.
 */
sl_status_t zwave_command_class_switch_color_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_SWITCH_COLOR_H
/** @} end zwave_command_class_switch_color */
