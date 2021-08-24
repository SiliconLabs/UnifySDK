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
 * @file zwave_command_class_wakeup.h
 * @brief TODO: Write brief for zwave_command_class_wakeup
 *
 * TODO: Write component description for zwave_command_class_wakeup
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_WAKEUP_H
#define ZWAVE_COMMAND_CLASS_WAKEUP_H

#include "attribute_store.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

sl_status_t zwave_command_class_wakeup_init();

bool zwave_command_class_wakeup_supports_wake_up_on_demand(
  attribute_store_node_t node_id_node);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_WAKEUP_H
/** @} end zwave_command_class_wakeup */
