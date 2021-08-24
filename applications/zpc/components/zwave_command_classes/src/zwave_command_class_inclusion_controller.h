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
 * @defgroup zwave_command_class_inclusion_controller Inclusion Controller Command Class
 * @ingroup command_classes
 * @brief The module handles the SIS side of the Inclusion Controller Command Class
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_INCLUSION_CONTROLLER_H
#define ZWAVE_COMMAND_CLASS_INCLUSION_CONTROLLER_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup fixture for @ref inclusion_controller_command_class
 *
 * This setup will register callbacks to the zwave controller and zwave
 * command handler modules, so that it can be notified when to parse commands or
 * issue commands the Z-Wave nodes.
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_inclusion_controller_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_INCLUSION_CONTROLLER_H
/** @} end zwave_command_class_inclusion_controller */
