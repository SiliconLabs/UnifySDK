/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup zwave_command_class_user_credential
 * @brief Sound Switch Command Class handlers and control function
 *
 * This module implement some of the functions to control the
 * Sound Switch Command Class
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_USER_CREDENTIAL_H
#define ZWAVE_COMMAND_CLASS_USER_CREDENTIAL_H

#include "sl_status.h"
#include "zwave_command_class_user_credential_types.h"
#include "attribute_store.h"

#ifdef __cplusplus
#include <string>
#include "sl_log.h"
extern "C" {
#endif


typedef void (*user_credential_slot_changed_callback_t)(
  const user_credential_credential_identifier_t old_credential_slot_id,
  const attribute_store_node_t new_credential_node);

void zwave_command_class_user_credential_set_uuic_slot_changed_callback(
  user_credential_slot_changed_callback_t callback);


sl_status_t zwave_command_class_user_credential_init();

#ifdef __cplusplus
} // extern "C"
typedef void (*user_credential_slot_message_callback_t)(
  sl_log_level level, const std::string& message);
void zwave_command_class_user_credential_set_message_callback(
  user_credential_slot_message_callback_t callback);
#endif




#endif  //ZWAVE_COMMAND_CLASS_USER_CREDENTIAL_H
        /** @} end zwave_command_class_user_credential */