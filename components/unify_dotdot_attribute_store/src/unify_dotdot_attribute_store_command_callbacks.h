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
 * @defgroup unify_dotdot_attribute_store_command_callbacks Command handlers for incoming MQTT UCL/DotDot commands
 * @ingroup unify_dotdot_attribute_store
 * @brief Registers command callbacks to DotDot MQTT for cluster commands.
 *
 * @{
 */

#ifndef UNIFY_DOTDOT_ATTRIBUTE_STORE_COMMAND_CALLBACKS_H
#define UNIFY_DOTDOT_ATTRIBUTE_STORE_COMMAND_CALLBACKS_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes callbacks to the DotDot MQTT component for the ZCL/DotDot
 * Attribute Store.
 *
 * @return SL_STATUS_OK on success, any other value in case of error.
 */
sl_status_t unify_dotdot_attribute_store_command_callbacks_init();

#ifdef __cplusplus
}
#endif

#endif  //UNIFY_DOTDOT_ATTRIBUTE_STORE_COMMAND_CALLBACKS_H
/** @} end unify_dotdot_attribute_store_command_callbacks */
