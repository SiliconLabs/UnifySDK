
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
 * @file zwave_command_class_multi_command.h
 * @defgroup multi_command_command_class Multi Command Command Class
 * @ingroup command_classes
 * @brief Multi Command Command Class handlers
 *
 * This module takes care of Multi Command Command Class support
 * list of attributes, as the association groups and their content are the same
 * resource, that can be modified with either Command Class
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_MULTI_COMMAND_H
#define ZWAVE_COMMAND_CLASS_MULTI_COMMAND_H

#include "sl_status.h"
#include "zwave_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Multi Command handler
 * 
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_multi_command_init(void);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_MULTI_COMMAND_H
/** @} end zwave_command_class_multi_command */
