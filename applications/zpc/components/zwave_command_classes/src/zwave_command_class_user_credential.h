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
extern "C" {
#endif

/**
 * @brief Send a new user command (User SET) to the end device 
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param user_id The unique ID of the user. Should NOT exists and NOT be 0
 * @param user_type The type of the user credential.
 * @param credential_rule The rule associated with the user credential.
 * @param user_active_state The active state of the user.
 * @param expiring_timeout The timeout for the user credential to expire, in minutes. Will be ignored unless user_type == 0x07.
 * @param user_name_encoding The encoding format of the user name.
 * @param user_name The name of the user.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the user_id already exists or is 0.
 */
sl_status_t zwave_command_class_user_credential_add_new_user(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t user_type,
  user_credential_rule_t credential_rule,
  user_credential_user_active_state_t user_active_state,
  user_credential_expiring_timeout_minutes_t expiring_timeout,
  user_credential_user_name_encoding_t user_name_encoding,
  const char *user_name);


/**
 * @brief Send a modify user command (User SET) to the end device 
 *
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param user_id The unique ID of the user. Should exists. Should NOT be 0.
 * @param user_type The type of the user credential.
 * @param credential_rule The rule associated with the user credential.
 * @param user_active_state The active state of the user.
 * @param expiring_timeout The timeout for the user credential to expire, in minutes. Will be ignored unless user_type == 0x07.
 * @param user_name_encoding The encoding format of the user name.
 * @param user_name The name of the user.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the user_id doesn't exists or is 0.
 */
sl_status_t zwave_command_class_user_credential_modify_user(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t user_type,
  user_credential_rule_t credential_rule,
  user_credential_user_active_state_t user_active_state,
  user_credential_expiring_timeout_minutes_t expiring_timeout,
  user_credential_user_name_encoding_t user_name_encoding,
  const char *user_name);

/**
 * @brief Send a delete user command (User SET) to the end device
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param user_id The unique ID of the user. Should exists.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the user_id doesn't exists.
*/
sl_status_t zwave_command_class_user_credential_delete_user(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id);

/**
 * @brief Send a new credential command (Credential SET) to the end device
 * 
 * The credential data expected is an UTF-8 string. Some check/conversion will be performed based on the credential type.
 * - PINCode  : Refuse any values that is not between '0' and '9'.
 * - Password : Will be stored in UTF-16 format. The exposed value still be in UTF-8, but the end device will receive UTF-16 data.
 * - Others   : Will be stored as is.
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param user_id The unique ID of the user. Should exists. Cannot be 0.
 * @param credential_type The type of the credential. Can exist or not. Cannot be 0.
 * @param credential_slot The slot of the credential. Should not exists. Cannot be 0.
 * @param credential_data Raw data of the credential. Checks are performed based on the credential type.
 * 
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the user_id doesn't exists or the credential_slot already exists or one of the value is 0. Also if the credential data is invalid.
 */
sl_status_t zwave_command_class_user_credential_add_new_credential(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  const char* credential_data);

/**
 * @brief Send a modify credential command (Credential SET) to the end device
 * 
 * The credential data expected is an UTF-8 string. Some check/conversion will be performed based on the credential type.
 * - PINCode  : Refuse any values that is not between '0' and '9'.
 * - Password : Will be stored in UTF-16 format. The exposed value still be in UTF-8, but the end device will receive UTF-16 data.
 * - Others   : Will be stored as is.
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param user_id The unique ID of the user. Should exists. Cannot be 0.
 * @param credential_type The type of the credential. Should exists. Cannot be 0.
 * @param credential_slot The slot of the credential. Should exists. Cannot be 0.
 * @param credential_data Raw data of the credential. Checks are performed based on the credential type.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the user_id, credential type or credential slot doesn't exists or one of the value is 0. Also if the credential data is invalid.
 */
sl_status_t zwave_command_class_user_credential_modify_credential(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  const char* credential_data
);

/**
 * @brief Send a delete credential command (Credential SET) to the end device
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param user_id The unique ID of the user. Should exists. Cannot be 0.
 * @param credential_type The type of the credential. Should exists. Cannot be 0.
 * @param credential_slot The slot of the credential. Should exists. Cannot be 0.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the user_id, credential type or credential slot doesn't exists or one of the value is 0.
*/
sl_status_t zwave_command_class_user_credential_delete_credential(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot);

sl_status_t zwave_command_class_user_credential_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_USER_CREDENTIAL_H
        /** @} end zwave_command_class_user_credential */