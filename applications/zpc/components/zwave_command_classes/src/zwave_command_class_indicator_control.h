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
 * @file zwave_command_class_indicator_control.h

 * @defgroup zwave_command_class_indicator Indicator  Command Class
 * @ingroup command_classes
 * @brief The module handles the control part of the indicator command class

 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_INDICATOR_CONTROL_H
#define ZWAVE_COMMAND_CLASS_INDICATOR_CONTROL_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Regisers this command class handler with the command class
 * handler framework
 * 
 * For details about the indicator attribute store datamodel see here
 * @ref COMMAND_CLASS_INDICATOR_handler
 * 
 * @return sl_status_t SL_STATUS_OK on succsess.
 */
sl_status_t zwave_command_class_indicator_control_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_INDICATOR_CONTROL_H
/** @} end zwave_command_class_indicator_control */
