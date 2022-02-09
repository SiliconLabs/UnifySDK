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
 * @defgroup zwave_command_class_manufacturer_specific_control Control of the 
 * Manufacturer Specifc command class
 * 
 * @ingroup command_classes
 * 
 * @brief The module handles the control part of the Manufacturer Specifc 
 * command class. The auto-generated command handlers are used for the main
 * part of the work.
 * 
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_MANUFACTURER_SPECIFIC_CONTROL_H
#define ZWAVE_COMMAND_CLASS_MANUFACTURER_SPECIFIC_CONTROL_H

#include "sl_status.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the control part of manufacturer specic
 * 
 * @return sl_status_t 
 */
sl_status_t zwave_command_class_manufacturer_specific_control_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_MANUFACTURER_SPECIFIC_CONTROL_H

/** @} end zwave_command_class_manufacturer_specific_control */