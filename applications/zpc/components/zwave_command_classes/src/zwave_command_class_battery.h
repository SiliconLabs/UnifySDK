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
 * @defgroup zwave_command_class_battery Battery Command Class control handler
 * @ingroup command_classes
 * @brief The module handles the control part of Battery Command Class
 *
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_BATTERY_H
#define ZWAVE_COMMAND_CLASS_BATTERY_H

#include "sl_status.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Registers this command class handler with the command class
 * handler framework
 *
 * For details about the Battery attribute store datamodel see here
 * @ref COMMAND_CLASS_BATTERY_handler
 *
 * @return sl_status_t SL_STATUS_OK on success.
 * 
 */
sl_status_t zwave_command_class_battery_control_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_BATTERY_H
/** @} end zwave_command_class_battery */
