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
 * @defgroup zwave_command_class_user_credential_api
 * @brief User Credential Command Class High Level API
 *
 * @see zwave_command_class_user_credential for interaction with zwave frames and end device
 *
 */

#ifndef ZWAVE_COMMAND_CLASS_USER_CREDENTIAL_API_H
#define ZWAVE_COMMAND_CLASS_USER_CREDENTIAL_API_H

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

/**
 * @brief Send a delete all users command (User SET) to the end device
 * 
 * It will send an User SET command with user id = 0 and operation type delete to remove all users.
 * (CC:0083.01.05.11.009)
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL otherwise
*/
sl_status_t zwave_command_class_user_credential_delete_all_users(
  attribute_store_node_t endpoint_node);

/**
 * @brief Send a delete all credentials command (Credential SET) to the end device
 * 
 * It will send an Credential SET command with user_id, credential_type and credential_slot = 0 and operation type delete to remove all credentials.
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL otherwise
*/
sl_status_t zwave_command_class_user_credential_delete_all_credentials(
  attribute_store_node_t endpoint_node);

/**
 * @brief Send a delete all credentials by type command (Credential SET) to the end device
 * 
 * It will send an Credential SET command with user_id = 0 and operation type delete to remove all credentials of the specified type.
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param credential_type The type of the credential. Should exists. Cannot be 0.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL otherwise
*/
sl_status_t
  zwave_command_class_user_credential_delete_all_credentials_by_type(
    attribute_store_node_t endpoint_node,
    user_credential_type_t credential_type);

/**
 * @brief Send a delete all credentials for user command (Credential SET) to the end device
 * 
 * It will send an Credential SET command with credential_type and credential_slot = 0 and operation type delete to remove all credentials of the specified user.
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param user_id The unique ID of the user. Should exists. Cannot be 0.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL otherwise
 * 
 * @note This function will not remove the user itself.
 */
sl_status_t zwave_command_class_user_credential_delete_all_credentials_for_user(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id);

/**
 * @brief Send a delete all credentials for user by type command (Credential SET) to the end device
 * 
 * It will send an Credential SET command with credential_slot = 0 and operation type delete to remove all credentials of the specified user and type.
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param user_id The unique ID of the user. Should exists. Cannot be 0.
 * @param credential_type The type of the credential. Should exists. Cannot be 0.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL otherwise
 * 
 * @note This function will not remove the user itself.
*/
sl_status_t
  zwave_command_class_user_credential_delete_all_credentials_for_user_by_type(
    attribute_store_node_t endpoint_node,
    user_credential_user_unique_id_t user_id,
    user_credential_type_t credential_type);


/**
 * @brief Send new credential command (Credential Learn Start) with operation type Add to the end device
 * 
 * @note The credential type must be supporting the Credential Learn feature.
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param user_id The unique ID of the user. Should exists. Cannot be 0.
 * @param credential_type The type of the credential. Can exist or not. Cannot be 0.
 * @param credential_slot The slot of the credential. Should not exists. Cannot be 0. 
 * @param credential_learn_timeout Timeout for each credential learn step on the node, in seconds. Can be set to 0 to use recommended timeout by the end device.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the user_id doesn't exists or the credential_slot already exists or one of the value is 0. Also if the credential type doesn't support the learn mode.
 */
sl_status_t zwave_command_class_user_credential_credential_learn_start_add(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  user_credential_learn_timeout_t credential_learn_timeout);

/**
 * @brief Send new credential command (Credential Learn Start) with operation type Modify to the end device
 * 
 * @note The credential type must be supporting the Credential Learn feature.
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param user_id The unique ID of the user. Should exists. Cannot be 0.
 * @param credential_type The type of the credential. Should exists. Cannot be 0.
 * @param credential_slot The slot of the credential. Should exists. Cannot be 0. 
 * @param credential_learn_timeout Timeout for each credential learn step on the node, in seconds. Can be set to 0 to use recommended timeout by the end device.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the user_id doesn't exists or the credential_slot/credential type is 0 or does not exists. Also if the credential type doesn't support the learn mode.
 */
sl_status_t zwave_command_class_user_credential_credential_learn_start_modify(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  user_credential_learn_timeout_t credential_learn_timeout);

/**
 * @brief Send new credential command (Credential Learn Stop) to the end device
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL otherwise
*/
sl_status_t zwave_command_class_user_credential_credential_learn_stop(
  attribute_store_node_t endpoint_node);


/**
 * @brief Send a User Unique Identifier Credential Association Set to the end device 
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param credential_type The type of the credential.
 * @param source_user_id The unique ID of the source user. Should exists. Cannot be 0.
 * @param source_credential_slot The slot of the source credential. Should exists. Cannot be 0.
 * @param destination_user_id The unique ID of the destination user. Cannot be 0.
 * @param destination_credential_slot The slot of the destination credential. Should not exists. Cannot be 0.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the user_id already exists or is 0.
 */
sl_status_t zwave_command_class_user_credential_uuic_association_set(
  attribute_store_node_t endpoint_node,
  user_credential_type_t credential_type,
  user_credential_user_unique_id_t source_user_id,
  user_credential_slot_t source_credential_slot,
  user_credential_user_unique_id_t destination_user_id,
  user_credential_slot_t destination_credential_slot);

/**
 * @brief Send a User Checksum Get command to the end device
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param user_id The unique ID of the source user. Should exists. Cannot be 0.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the user_id doesn't exists or is 0.
 */
sl_status_t zwave_command_class_user_credential_get_user_checksum(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id);


/**
 * @brief Send a Credential Checksum Get command to the end device
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param credential_type The type of the credential. Cannot be 0
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the credential_type is not supported or is 0.
 */
sl_status_t zwave_command_class_user_credential_get_credential_checksum(
  attribute_store_node_t endpoint_node,
  user_credential_type_t credential_type);


/**
 * @brief Send a All Users Checksum Get command to the end device
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL otherwise
 */
sl_status_t zwave_command_class_user_credential_get_all_users_checksum(
  attribute_store_node_t endpoint_node);

/**
 * @brief Send a Set Admin Pin Code command to the end device
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param credential_data The new admin pin code. Must contains only digits.
 * 
 * @return sl_status_t SL_STATUS_OK if the attribute store was updated successfully
 * @return sl_status_t SL_STATUS_FAIL if the credential_data is invalid.
 */
sl_status_t zwave_command_class_user_credential_set_admin_pin_code(
  attribute_store_node_t endpoint_node,
   const char* credential_data);

/**
 * @brief Return true if the User Credential Command Class supports the Admin Pin Code Deactivation command
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * 
 * @return true if the command is supported
 * @return false if the command is not supported
 */
bool zwave_command_class_user_credential_supports_admin_pin_code_deactivation(
  attribute_store_node_t endpoint_node);

/**
 * @brief Test if given command id is supported by the User Credential Command Class
 * 
 * It will perform a check first to see if given node is supporting the User Credential Command Class.
 * If not it will return false.
 * 
 * @param endpoint_node The attribute store node of the endpoint where the user credential is located.
 * @param command_id The command id to test
 * 
 * @return true if the command is supported
 * @return false if the command is not supported
 */
bool zwave_command_class_user_credential_supports(
  attribute_store_node_t endpoint_node, uint8_t command_id);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_USER_CREDENTIAL_API_H
        /** @} end zwave_command_class_user_credential */